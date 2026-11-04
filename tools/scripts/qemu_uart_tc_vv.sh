#!/usr/bin/env bash
# frick-fsw - external UART telecommand / telemetry path (T2 §3)
#
# Delivers the GSW golden test telecommand on QEMU's serial interface (stdin),
# captures everything the image writes to the same interface, and verifies the
# captured telemetry with tools/scripts/uart_tm_verify.py.
#
# This is the REAL external path: the TC bytes cross QEMU's serial device. There
# is no in-image injection, no mock link and no test seam. Whether the image is
# able to answer depends on the watchdog condition, which is not touched here.
#
# Expected outcome today: the image aborts at watchdog init (NC-PAL-01 /
# NC-EXT-198), so no telemetry is produced and the verifier FAILS. The script
# records that as the real result; it is not a PASS, not BLOCKED-by-skip and not
# a mock-coverage claim.
#
# Three results are decided independently and PASS requires all of them:
#   * the QEMU process code (accepted: 0, or 124 as the bounded-capture timeout);
#   * the console (no [FF-PANIC] / RTEMS_FATAL_SOURCE_EXIT / EXIT STATUS NOT ZERO,
#     scanned by the shared target_run_verdict.scan_fatal);
#   * the U1 telemetry verifier.
# A U1 capture that arrives together with a panic block is therefore a FAILURE,
# and the two component results are always reported separately.
#
# Test seam (disclosed, N10-M3): the gated overrides T2_QEMU_CMD / T2_RUN_ROOT /
# T2_SKIP_PREREQ / T2_ELF exist only for the offline fixtures and are refused
# unless T2_ALLOW_QEMU_STUB=1. The QEMU command boots "${ELF#${ROOT}/}", i.e. the
# same path that is hashed, so the advertised elf_sha256 is the booted image.
#
# Exit semantics: 0 = all three results clean; 1 = QEMU process failure, console
# fatal, or no/invalid U1 telemetry (including the watchdog abort); 2 = missing
# prerequisite (image or ELF). Never 0 for "nothing happened".
#
# Requires the ELF produced by tools/scripts/qemu_req_vv.sh (or an equivalent
# build of the test-only image); it does not build, so it cannot pull anything.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="frick-rtems-toolchain:local"
ELF="${ROOT}/build/t1-qemu/frick_fsw_reqtest"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"

# Test-only overrides for the fail-closed regression fixture: refused unless
# T2_ALLOW_QEMU_STUB=1 is set explicitly, so a normal invocation can never
# substitute the emulator or skip the prerequisites by accident.
if [[ "${T2_ALLOW_QEMU_STUB:-0}" == "1" ]]; then
  RUN_PREFIX="-stub"
  RUN_MODE="stub"
  QEMU_CMD="${T2_QEMU_CMD:-}"
  RUN_ROOT="${T2_RUN_ROOT:-${ROOT}/build}"
  SKIP_PREREQ="${T2_SKIP_PREREQ:-0}"
  if [[ -n "${T2_ELF:-}" ]]; then
    ELF="${T2_ELF}"   # test-only: path to the image under test
  fi
else
  if [[ -n "${T2_QEMU_CMD:-}" || -n "${T2_RUN_ROOT:-}" || -n "${T2_SKIP_PREREQ:-}" || -n "${T2_ELF:-}" ]]; then
    echo "[t2-uart] REFUSED: T2_QEMU_CMD/T2_RUN_ROOT/T2_SKIP_PREREQ/T2_ELF require T2_ALLOW_QEMU_STUB=1 (test-only)" >&2
    exit 2
  fi
  QEMU_CMD=""
  RUN_ROOT="${ROOT}/build"
  SKIP_PREREQ=0
  RUN_PREFIX=""
  RUN_MODE="real"
fi
RUN_DIR="${RUN_ROOT}/t2-uart${RUN_PREFIX}-${STAMP}"
CAPTURE="${RUN_DIR}/capture.bin"

# GSW golden TC (tools/gsw/ff_gsw.py, tests/python/test_ff_gsw.py).
GOLDEN_TC_HEX="18 10 C0 00 00 0A 2B 11 01 01 02 01 02 03 04 F7 2B"
# Seconds to wait after boot before injecting the frame, and the capture window.
DELAY_S="${T2_TC_DELAY_S:-8}"
WINDOW_S="${T2_WINDOW_S:-20}"

mkdir -p "${RUN_DIR}"
echo "[t2-uart] run directory: ${RUN_DIR} (mode: ${RUN_MODE})"

# The producing tool's identity is sampled at start AND at end: bash reads a
# script incrementally, so a mid-run edit would otherwise yield a record that
# does not describe the executed logic (N10 review R2-05).
TOOL_SHA_START="$(sha256sum "${BASH_SOURCE[0]}" | cut -d' ' -f1)"
printf '%s\n' "${TOOL_SHA_START}" > "${RUN_DIR}/producing-tool.sha256"

if [[ "${SKIP_PREREQ}" == "1" ]]; then
  IMAGE_ID="(test-only stub run: no image)"
else
  if ! docker image inspect "${IMG}" >/dev/null 2>&1; then
    echo "[t2-uart] MISSING PREREQUISITE: docker image ${IMG} is not present (no pull attempted)" >&2
    exit 2
  fi
  IMAGE_ID="$(docker image inspect -f '{{.Id}}' "${IMG}")"
fi
if [[ ! -f "${ELF}" ]]; then
  echo "[t2-uart] MISSING PREREQUISITE: ${ELF} not built (run tools/scripts/qemu_req_vv.sh)" >&2
  exit 2
fi
ELF_SHA="$(sha256sum "${ELF}" | cut -d' ' -f1)"
NONCE="$(head -c 16 /dev/urandom | od -An -tx1 | tr -d ' \n')"
printf '%s  %s\n' "${ELF_SHA}" "${ELF}" > "${RUN_DIR}/elf.sha256"
printf '%s\n' "${NONCE}" > "${RUN_DIR}/run-nonce.txt"
echo "[t2-uart] image ${IMAGE_ID}"
echo "[t2-uart] ELF sha256 ${ELF_SHA}"
echo "[t2-uart] nonce ${NONCE}"
echo "[t2-uart] injecting the golden TC after ${DELAY_S}s, capture window ${WINDOW_S}s"

# Feed the TC on stdin after the delay; QEMU writes the UART console to stdout.
# Accepted QEMU process codes: 0, and 124 only as the bounded-capture timeout.
# Anything else is a QEMU process failure even if the captured bytes look valid.
set +e
if [[ -n "${QEMU_CMD}" ]]; then
  echo "[t2-uart] TEST-ONLY: QEMU command overridden by T2_QEMU_CMD"
  bash -c "${QEMU_CMD}" > "${CAPTURE}" 2>&1
  QEMU_RC=$?
else
  {
    sleep "${DELAY_S}"
    printf '%s' "${GOLDEN_TC_HEX}" | xxd -r -p
    sleep "$((WINDOW_S - DELAY_S > 0 ? WINDOW_S - DELAY_S : 1))"
  } | timeout "$((WINDOW_S + 5))" docker run --rm -i --pull=never --network=none \
        --user "$(id -u):$(id -g)" \
        -v "${ROOT}:/workspace" -w /workspace \
        "${IMG}" \
        bash -lc "qemu-system-riscv64 -M virt -nographic -bios ${ELF#${ROOT}/}" \
        > "${CAPTURE}" 2>&1
  QEMU_RC=$?
fi
set -e
echo "[t2-uart] qemu exit: ${QEMU_RC} (accepted codes: 0, or 124 as the bounded-capture timeout)"
printf '%s\n' "${QEMU_RC}" > "${RUN_DIR}/qemu-exit-code.txt"
echo "[t2-uart] captured $(wc -c < "${CAPTURE}") bytes"

set +e
python3 "${ROOT}/tools/scripts/uart_tm_verify.py" \
  --capture "${CAPTURE}" \
  --json "${RUN_DIR}/uart-verify.json" | tee "${RUN_DIR}/uart-verify.log"
VERIFY_RC="${PIPESTATUS[0]}"
set -e
echo "[t2-uart] verifier exit: ${VERIFY_RC}"

# Two independent results: the QEMU process code and the console (fatal scan,
# shared with the T1 verdict so the marker grammar has one definition), plus the
# U1 verifier. PASS requires all three to be clean.
TOOL_SHA_END="$(sha256sum "${BASH_SOURCE[0]}" | cut -d' ' -f1)"
if [[ "${TOOL_SHA_END}" != "${TOOL_SHA_START}" ]]; then
  echo "[t2-uart] FAIL: the producing tool changed during the run (${TOOL_SHA_START} -> ${TOOL_SHA_END}); the record cannot be attributed." >&2
  exit 1
fi
ELF_SHA_AFTER="$(sha256sum "${ELF}" | cut -d' ' -f1)"
if [[ "${ELF_SHA_AFTER}" != "${ELF_SHA}" ]]; then
  echo "[t2-uart] FAIL: the image changed between hashing and the run (${ELF_SHA} -> ${ELF_SHA_AFTER})." >&2
  exit 1
fi

set +e
python3 - "${ROOT}" "${RUN_DIR}" "${QEMU_RC}" "${VERIFY_RC}" "${NONCE}" "${ELF_SHA}" \
          "${BASH_SOURCE[0]}" "${RUN_MODE}" "${IMAGE_ID}" "${ELF}" <<'PYMETA'
import hashlib, json, os, sys
root, run_dir, qemu_rc, verify_rc, nonce, elf_sha, script, run_mode, image_id, elf = sys.argv[1:11]
sys.path.insert(0, os.path.join(root, "tools", "scripts"))
import target_run_verdict as trv  # shared fatal-marker scan (single definition)

qemu_exit = int(qemu_rc)
qemu_ok = qemu_exit in (0, 124)
capture_path = os.path.join(run_dir, "capture.bin")
capture = open(capture_path, "rb").read() if os.path.exists(capture_path) else b""
fatal = trv.scan_fatal(capture.decode("utf-8", errors="replace"))
watchdog_abort = b"watchdog: init failed" in capture
meta = {
    "run_nonce": nonce,
    "producing_tool": os.path.basename(script),
    "producing_tool_sha256": hashlib.sha256(open(script, "rb").read()).hexdigest(),
    "run_mode": run_mode,
    "docker_image_id": image_id,
    "elf_path": elf,
    "elf_sha256": elf_sha,
    "bios_arg": os.path.relpath(elf, root) if elf.startswith(root + os.sep) else elf,
    "capture_sha256": hashlib.sha256(capture).hexdigest() if capture else None,
    "capture_bytes": len(capture),
    "qemu_exit": qemu_exit,
    "qemu_accepted_codes": [0, 124],
    "qemu_ok": qemu_ok,
    "console_fatal": fatal,
    "console_ok": fatal["ok"],
    "watchdog_abort": watchdog_abort,
    "verifier_exit": int(verify_rc),
    "component_u1_ok": int(verify_rc) == 0,
    "result": "PASS" if (qemu_ok and fatal["ok"] and int(verify_rc) == 0) else "FAIL",
}
with open(os.path.join(run_dir, "uart-verdict.json"), "w", encoding="utf-8") as handle:
    json.dump(meta, handle, indent=2, sort_keys=True)
    handle.write("\n")
print(f"[t2-uart] producing tool {meta['producing_tool']} sha256 {meta['producing_tool_sha256']}")
print(f"[t2-uart] console fatal markers: {len(fatal['hits'])} (console_ok={fatal['ok']})")
if not fatal["ok"]:
    for hit in fatal["hits"]:
        print(f"[t2-uart]   F: line {hit['line']}: {hit['marker']} :: {hit['text']}")
sys.exit(0 if meta["result"] == "PASS" else 1)
PYMETA
VERDICT_RC=$?
set -e
echo "[t2-uart] composite verdict exit: ${VERDICT_RC}"

QEMU_OK=0
if [[ "${QEMU_RC}" -eq 0 || "${QEMU_RC}" -eq 124 ]]; then
  QEMU_OK=1
fi

if [[ "${QEMU_OK}" -ne 1 ]]; then
  echo "[t2-uart] RESULT: FAIL (QEMU PROCESS FAILURE: exit ${QEMU_RC}; accepted codes are 0 and 124)." >&2
  echo "[t2-uart] The verifier result above (exit ${VERIFY_RC}) is the component result only and cannot override this failure." >&2
  exit 1
fi

if [[ "${VERDICT_RC}" -ne 0 && "${VERIFY_RC}" -eq 0 ]]; then
  echo "[t2-uart] RESULT: FAIL (WHOLE-RUN: the console contains a panic/fatal marker although the U1 set was captured)." >&2
  if grep -aq "watchdog: init failed" "${CAPTURE}"; then
    echo "[t2-uart] Cause: the image aborts at watchdog init (NC-PAL-01 / NC-EXT-198); a component U1 result cannot override it." >&2
  fi
  exit 1
fi

if [[ "${VERIFY_RC}" -ne 0 ]]; then
  echo "[t2-uart] RESULT: FAIL (no valid U1 telemetry on the external path)." >&2
  if grep -aq "watchdog: init failed" "${CAPTURE}"; then
    echo "[t2-uart] Cause: the image aborts at watchdog init (NC-PAL-01 / NC-EXT-198); egress only - BLOCKED, not target PASS." >&2
  fi
  exit 1
fi

echo "[t2-uart] RESULT: PASS (U1 telemetry set verified over the external serial path)"
exit 0
