#!/usr/bin/env bash
# frick-fsw - XW-3: real external TC on the serial input -> unchanged middleware
#             -> captured TM verified by an independent oracle (TEST-ONLY image)
#
# This runner drives the ACCEPTED XW-2 test-only image (`frick_fsw_xw2`, CMake
# option `FF_ENABLE_EXTWD_TARGET_TEST`, default OFF). Nothing under `src/` or
# `config/` is changed by this increment; the image is the same binary that the
# XW-2 round produced, rebuilt from the same sources.
#
# What is real here: the telecommand octets are written to QEMU's serial input.
# They cross QEMU's serial device, are read from /dev/console by the unchanged
# `link_uart` backend, extracted into a CCSDS frame, handed to the unchanged
# `CMD_INGEST` application and dispatched by the unchanged PUS layer; the
# responses are written by the unchanged `link_uart` send path to the same
# serial device and captured by this runner. There is no in-image injection, no
# direct PUS call, no echoed fixture and no mock link.
#
# QEMU invocation note (measured, not assumed): `-nographic` selects the QEMU
# stdio *mux*, whose Ctrl-A escape processing silently drops octets of this
# frame (0x01/0x02 sequences in the TC source id / user data). This runner uses
# `-display none -monitor none -serial stdio`, i.e. the serial device attached
# directly to stdio, and the frame arrives octet-exact. See the handover for the
# corresponding finding on the earlier T2 runner (NC-XW3-01).
#
# Cases and negative controls are declared in tools/scripts/xw3_manifest.json;
# the oracle and the whole-run verdict are in tools/scripts/xw3_verdict.py.
# `tools/scripts/xw3_negative_controls.py` drives the negative controls through
# THIS runner and THAT verdict (fixture mode, refused unless explicitly enabled).
#
# Scope, stated plainly: this is the real external byte path on a TEST-ONLY
# emulated-target image whose watchdog device seam is substituted by the
# validated model. It is not a device, not HIL, not N18, not all-I/O, not a
# timing/WCET result, not a mission-profile result, and not a reset or
# independence result. NC-PAL-01 / NC-EXT-198 remain OPEN.
#
# Exit: 0 = every required case PASS and every observation recorded and the
# whole-run summary clean; 1 = any verification failure; 2 = missing
# prerequisite or usage error. Never 0 for "nothing happened".
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="frick-rtems-toolchain:local"
BUILD="${ROOT}/build/xw3-qemu"
MANIFEST="${ROOT}/tools/scripts/xw3_manifest.json"
VERDICT="${ROOT}/tools/scripts/xw3_verdict.py"
ELF_NAME="frick_fsw_xw2"
CONTROL_ELF_NAME="frick_fsw_xw2_nowrap"

GOLDEN_HEX="18 10 C0 00 00 0A 2B 11 01 01 02 01 02 03 04 F7 2B"
CORRUPT_HEX="18 10 C0 00 00 0A 2B 11 01 01 02 01 02 03 04 F7 2A"
SPLIT_HEAD="18 10 C0 00 00 0A 2B 11"
SPLIT_TAIL="01 01 02 01 02 03 04 F7 2B"

DELAY_S="${XW3_DELAY_S:-6}"
WINDOW_S="${XW3_WINDOW_S:-18}"

ONLY_CASE=""
NO_BUILD=0
FIXTURE_DIR=""
FIXTURE_CASE=""
FIXTURE_MODE=""

usage() {
  cat >&2 <<'TXT'
usage: qemu_xw3_vv.sh [--no-build] [--only CASE]
       qemu_xw3_vv.sh --fixture DIR --case CASE --mode accept|reject|observe
                      (requires XW3_ALLOW_FIXTURE=1)
TXT
  exit 2
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-build) NO_BUILD=1; shift ;;
    --only) ONLY_CASE="${2:-}"; [[ -n "${ONLY_CASE}" ]] || usage; shift 2 ;;
    --fixture) FIXTURE_DIR="${2:-}"; [[ -n "${FIXTURE_DIR}" ]] || usage; shift 2 ;;
    --case) FIXTURE_CASE="${2:-}"; [[ -n "${FIXTURE_CASE}" ]] || usage; shift 2 ;;
    --mode) FIXTURE_MODE="${2:-}"; [[ -n "${FIXTURE_MODE}" ]] || usage; shift 2 ;;
    -h|--help) usage ;;
    *) echo "[xw3] unknown argument: $1" >&2; usage ;;
  esac
done

STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
if [[ -n "${FIXTURE_DIR}" ]]; then
  RUN_BASE="${ROOT}/build/xw3-fixture-${STAMP}"
else
  RUN_BASE="${ROOT}/build/xw3-run-${STAMP}"
fi
# Never reuse a directory: two invocations in the same UTC second must not
# overwrite each other's retained verdicts (reviewer finding Minor-4).
RUN_DIR="${RUN_BASE}"
RUN_SUFFIX=1
while [[ -e "${RUN_DIR}" ]]; do
  RUN_DIR="${RUN_BASE}-r${RUN_SUFFIX}"
  RUN_SUFFIX=$((RUN_SUFFIX + 1))
done
mkdir -p "${RUN_DIR}/cases"

echo "[xw3] run directory: ${RUN_DIR}"

# --------------------------------------------------------------------------
# prerequisites (fail closed; exit 2 is never a pass)
# --------------------------------------------------------------------------
if ! docker image inspect "${IMG}" >/dev/null 2>&1; then
  echo "[xw3] MISSING PREREQUISITE: docker image ${IMG} is not present (no pull attempted)" >&2
  exit 2
fi
IMAGE_ID="$(docker image inspect -f '{{.Id}}' "${IMG}")"
for required in "${MANIFEST}" "${VERDICT}"; do
  if [[ ! -f "${required}" ]]; then
    echo "[xw3] MISSING PREREQUISITE: ${required}" >&2
    exit 2
  fi
done
command -v python3 >/dev/null 2>&1 || { echo "[xw3] MISSING PREREQUISITE: python3" >&2; exit 2; }

# --------------------------------------------------------------------------
# fixture mode: the control seam. Refused unless explicitly enabled, and never
# used by the positive path. The capture/exit/metadata come from DIR; the
# verdict path below is the SAME code path as for a real run.
# --------------------------------------------------------------------------
if [[ -n "${FIXTURE_DIR}" ]]; then
  if [[ "${XW3_ALLOW_FIXTURE:-0}" != "1" ]]; then
    echo "[xw3] REFUSED: --fixture requires XW3_ALLOW_FIXTURE=1 (test-only negative controls)" >&2
    exit 2
  fi
  if [[ -z "${FIXTURE_CASE}" || -z "${FIXTURE_MODE}" ]]; then
    echo "[xw3] MISSING PREREQUISITE: --fixture needs --case and --mode" >&2
    exit 2
  fi
  if [[ ! -d "${FIXTURE_DIR}" ]]; then
    echo "[xw3] MISSING PREREQUISITE: fixture directory ${FIXTURE_DIR}" >&2
    exit 2
  fi
fi

# --------------------------------------------------------------------------
# build the test-only image (and its no-wrap control) from the current sources
# --------------------------------------------------------------------------
ELF="${BUILD}/${ELF_NAME}"
CONTROL_ELF="${BUILD}/${CONTROL_ELF_NAME}"

if [[ -n "${FIXTURE_DIR}" ]]; then
  # A fixture run must still be bound to a real image identity: use the image
  # built for this repository (or fail closed).
  if [[ ! -f "${ELF}" ]]; then
    echo "[xw3] MISSING PREREQUISITE: ${ELF} (build the image first: qemu_xw3_vv.sh)" >&2
    exit 2
  fi
elif [[ "${NO_BUILD}" -eq 0 ]]; then
  echo "[xw3] building ${ELF_NAME} (offline, cached toolchain image)"
  set +e
  docker run --rm --pull=never --network=none \
    --user "$(id -u):$(id -g)" -v "${ROOT}:/workspace" -w /workspace "${IMG}" \
    bash -lc "cmake -S . -B build/xw3-qemu -G Ninja -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
        -DFF_PLATFORM=rtems-generic -DFF_ENABLE_EXTWD_TARGET_TEST=ON \
        -DFF_ENABLE_TARGET_REQ_TEST=OFF -DFF_ENABLE_RENODE_TEST=OFF -DFF_USE_PUS_ASN1=ON && \
      cmake --build build/xw3-qemu -j\$(nproc)" 2>&1 | tee "${RUN_DIR}/build.log"
  BUILD_RC="${PIPESTATUS[0]}"
  set -e
  echo "[xw3] build exit: ${BUILD_RC}"
  if [[ "${BUILD_RC}" -ne 0 ]]; then
    echo "[xw3] FAIL: the test-only image did not build (log ${RUN_DIR}/build.log)" >&2
    exit 1
  fi
fi
if [[ ! -f "${ELF}" ]]; then
  echo "[xw3] MISSING PREREQUISITE: ${ELF} was not produced" >&2
  exit 2
fi
[[ -f "${BUILD}/CMakeCache.txt" ]] && cp "${BUILD}/CMakeCache.txt" "${RUN_DIR}/CMakeCache.txt"

# --------------------------------------------------------------------------
# identity binding (sources, tools, image, ELF, nonce)
# --------------------------------------------------------------------------
ELF_SHA="$(sha256sum "${ELF}" | cut -d' ' -f1)"
printf '%s  %s\n' "${ELF_SHA}" "${ELF}" > "${RUN_DIR}/elf.sha256"
NONCE="$(head -c 16 /dev/urandom | od -An -tx1 | tr -d ' \n')"
printf '%s\n' "${NONCE}" > "${RUN_DIR}/run-nonce.txt"

python3 - "${ROOT}" "${RUN_DIR}" "${IMAGE_ID}" "${ELF_SHA}" "${MANIFEST}" \
          "${VERDICT}" "${BASH_SOURCE[0]}" "${NONCE}" "${FIXTURE_DIR}" <<'PYID'
import hashlib, json, sys, os
root, run_dir, image_id, elf_sha, manifest, verdict, runner, nonce, fixture = sys.argv[1:10]
def sha(path):
    return hashlib.sha256(open(path, "rb").read()).hexdigest()
sources = [
    "CMakeLists.txt",
    "tests/target/fsw_main_xw2.c",
    "tests/target/app_xw2_test.c",
    "tests/target/app_xw2_test.h",
    "tests/stubs/ff_test_extwd_model.c",
    "tests/stubs/ff_test_extwd_model.h",
    "tests/stubs/ff_test_extwd_pal.c",
    "src/transport/link/link_uart.c",
    "src/apps/app_cmd_ingest.c",
    "src/pus/pus.c",
    "src/pus/pus_services.c",
]
meta = {
    "run_nonce": nonce,
    "mode": "fixture" if fixture else "real",
    "docker_image_id": image_id,
    "elf_sha256": elf_sha,
    "manifest_sha256": sha(manifest),
    "oracle_tool": os.path.basename(verdict),
    "oracle_tool_sha256": sha(verdict),
    "runner": os.path.basename(runner),
    "runner_sha256": sha(runner),
    "sources": {p: sha(os.path.join(root, p)) for p in sources},
}
with open(os.path.join(run_dir, "run-index.json"), "w", encoding="utf-8") as handle:
    json.dump(meta, handle, indent=2, sort_keys=True)
    handle.write("\n")
print(f"[xw3] image {image_id}")
print(f"[xw3] ELF sha256 {elf_sha}")
print(f"[xw3] run nonce {nonce}")
print(f"[xw3] manifest sha256 {meta['manifest_sha256']}")
print(f"[xw3] oracle tool sha256 {meta['oracle_tool_sha256']}")
PYID

# --------------------------------------------------------------------------
# one QEMU run: inject the planned octets, capture the console
# --------------------------------------------------------------------------
run_qemu() { # $1 = image path, $2 = capture path, $3 = plan ("delay|hex" lines)
  local image="$1" capture="$2" plan="$3"
  local rc=0
  set +e
  {
    local t=0 d h
    while IFS='|' read -r d h; do
      [[ -z "${d}" ]] && continue
      if (( d > t )); then sleep "$(( d - t ))"; fi
      t="${d}"
      printf '%s' "${h}" | xxd -r -p
    done <<< "${plan}"
    if (( WINDOW_S > t )); then sleep "$(( WINDOW_S - t ))"; fi
  } | timeout "$(( WINDOW_S + 8 ))" docker run --rm -i --pull=never --network=none \
        --user "$(id -u):$(id -g)" -v "${ROOT}:/workspace" -w /workspace "${IMG}" \
        bash -lc "qemu-system-riscv64 -M virt -display none -monitor none -serial stdio -bios ${image#${ROOT}/}" \
        > "${capture}" 2>&1
  rc=$?
  set -e
  echo "${rc}"
}

plan_for() { # $1 = variant -> plan lines on stdout
  case "$1" in
    golden) echo "${DELAY_S}|${GOLDEN_HEX}" ;;
    corrupt_crc) echo "${DELAY_S}|${CORRUPT_HEX}" ;;
    duplicate) echo "${DELAY_S}|${GOLDEN_HEX}"$'\n'"$(( DELAY_S + 2 ))|${GOLDEN_HEX}" ;;
    split) echo "${DELAY_S}|${SPLIT_HEAD}"$'\n'"$(( DELAY_S + 2 ))|${SPLIT_TAIL}" ;;
    *) echo "[xw3] internal error: unknown variant $1" >&2; exit 2 ;;
  esac
}

tc_for() { # $1 = variant -> the frame the oracle must derive the request from
  case "$1" in
    golden|duplicate|split) echo "${GOLDEN_HEX}" ;;
    corrupt_crc) echo "${CORRUPT_HEX}" ;;
    *) echo "[xw3] internal error: unknown variant $1" >&2; exit 2 ;;
  esac
}

case_meta() { # $1 = case dir, $2 = case name, $3 = variant, $4 = qemu exit, $5 = capture
  python3 - "$1" "$2" "$3" "$4" "$5" "${NONCE}" "${ELF_SHA}" <<'PYCASE'
import hashlib, json, os, sys, datetime
case_dir, case, variant, qemu_exit, capture, nonce, elf_sha = sys.argv[1:8]
data = open(capture, "rb").read()
meta = {
    "case": case,
    "variant": variant,
    "run_nonce": nonce,
    "elf_sha256": elf_sha,
    "capture_sha256": hashlib.sha256(data).hexdigest(),
    "capture_bytes": len(data),
    "qemu_exit": int(qemu_exit),
    "qemu_accepted_codes": [0, 124],
    "checked_at_utc": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
}
with open(os.path.join(case_dir, "run-meta.json"), "w", encoding="utf-8") as handle:
    json.dump(meta, handle, indent=2, sort_keys=True)
    handle.write("\n")
PYCASE
}

verify_case() { # $1 dir, $2 case, $3 variant, $4 mode, $5 qemu exit, [$6 tc hex]
  local dir="$1" case="$2" variant="$3" mode="$4" qemu_exit="$5"
  local tc_hex="${6:-$(tc_for "${variant}")}"
  local caller_errexit=0
  case "$-" in *e*) caller_errexit=1 ;; esac
  local capture="${dir}/capture.bin"
  if [[ ! -f "${capture}" ]]; then
    echo "[xw3] FAIL: ${case}: capture missing" >&2
    return 1
  fi
  # Real runs: the runner owns the metadata. Fixture runs: the fixture owns it
  # (a missing/mismatched fixture metadata is exactly what some controls test).
  if [[ -z "${FIXTURE_DIR}" ]]; then
    printf '%s\n' "${qemu_exit}" > "${dir}/qemu-exit.txt"
    case_meta "${dir}" "${case}" "${variant}" "${qemu_exit}" "${capture}"
  fi
  set +e
  python3 "${VERDICT}" verify \
    --capture "${capture}" \
    --tc "${tc_hex}" \
    --mode "${mode}" \
    --run-meta "${dir}/run-meta.json" \
    --nonce "${NONCE}" \
    --elf-sha256 "${ELF_SHA}" \
    --json "${dir}/verdict.json" | tee "${dir}/verdict.log"
  local rc="${PIPESTATUS[0]}"
  if [[ "${caller_errexit}" -eq 1 ]]; then set -e; fi
  printf '%s\n' "${rc}" > "${dir}/verdict-exit.txt"
  return "${rc}"
}

run_real_case() { # $1 = case name, $2 = variant, $3 = mode
  local case="$1" variant="$2" mode="$3"
  local dir="${RUN_DIR}/cases/${case}"
  mkdir -p "${dir}"
  local plan; plan="$(plan_for "${variant}")"
  printf '%s\n' "${plan}" > "${dir}/inject-plan.txt"
  echo "[xw3] ${case}: running (variant ${variant}, mode ${mode})"
  local rc; rc="$(run_qemu "${ELF}" "${dir}/capture.bin" "${plan}")"
  echo "[xw3] ${case}: qemu exit ${rc} (accepted 0/124)"
  verify_case "${dir}" "${case}" "${variant}" "${mode}" "${rc}"
}

# --------------------------------------------------------------------------
# fixture mode path (one case, then summary)
# --------------------------------------------------------------------------
if [[ -n "${FIXTURE_DIR}" ]]; then
  DIR="${RUN_DIR}/cases/${FIXTURE_CASE}"
  mkdir -p "${DIR}"
  for artifact in capture.bin qemu-exit.txt run-meta.json; do
    if [[ -f "${FIXTURE_DIR}/${artifact}" ]]; then
      cp "${FIXTURE_DIR}/${artifact}" "${DIR}/${artifact}"
    fi
  done
  if [[ ! -f "${FIXTURE_DIR}/tc.txt" ]]; then
    echo "[xw3] MISSING PREREQUISITE: fixture ${FIXTURE_DIR}/tc.txt" >&2
    exit 2
  fi
  cp "${FIXTURE_DIR}/tc.txt" "${DIR}/tc.txt"
  FIXTURE_TC="$(cat "${DIR}/tc.txt")"
  QEMU_EXIT="$(cat "${DIR}/qemu-exit.txt" 2>/dev/null || echo "missing")"
  # Explicit, documented rebinding for the POSITIVE fixture replay only: the
  # fixture capture is claimed to belong to THIS run. Each field is rebound only
  # when its flag is set; the negative controls leave at least one un-rebound so
  # that the mismatch path is exercised.
  if [[ -f "${DIR}/run-meta.json" ]] && [[ "${XW3_FIXTURE_REBIND:-0}" == "1" ]]; then
    python3 - "${DIR}/run-meta.json" "${NONCE}" "${ELF_SHA}" "${DIR}/capture.bin" \
              "${XW3_FIXTURE_REBIND_NONCE:-1}" "${XW3_FIXTURE_REBIND_ELF:-1}" <<'PYREBIND'
import hashlib, json, sys
path, nonce, elf, capture, rebind_nonce, rebind_elf = sys.argv[1:7]
meta = json.load(open(path, encoding="utf-8"))
if rebind_nonce == "1":
    meta["run_nonce"] = nonce
if rebind_elf == "1":
    meta["elf_sha256"] = elf
data = open(capture, "rb").read()
meta["capture_sha256"] = hashlib.sha256(data).hexdigest()
meta["capture_bytes"] = len(data)
json.dump(meta, open(path, "w", encoding="utf-8"), indent=2, sort_keys=True)
open(path, "a", encoding="utf-8").write("\n")
PYREBIND
  fi
  set +e
  verify_case "${DIR}" "${FIXTURE_CASE}" golden "${FIXTURE_MODE}" "${QEMU_EXIT}" "${FIXTURE_TC}"
  VC="$?"
  set -e
  # Stage 1: case-only summary (no gate metadata) -> summary.case.json
  set +e
  python3 "${VERDICT}" summary --run-dir "${RUN_DIR}" --manifest "${MANIFEST}" \
    --json "${RUN_DIR}/summary.case.json" | tee "${RUN_DIR}/summary.log"
  SUMMARY_RC="${PIPESTATUS[0]}"
  set -e
  echo "[xw3] summary exit: ${SUMMARY_RC}"

  # Fixture mode: gate reason only from case verdict or summary failure
  GATE_REASON=""
  if [[ "${VC}" -ne 0 ]]; then
    GATE_REASON="case_verdict_failed"
  elif [[ "${SUMMARY_RC}" -ne 0 ]]; then
    GATE_REASON="case_verdict_failed"
  fi
  # Stage 2: PYGATE reads staged case summary, adds gate metadata, atomically publishes final summary.json
  python3 - "${RUN_DIR}" "${GATE_REASON}" "${NONCE}" "${ELF_SHA}" "${VERDICT}" <<'PYGATE'
import hashlib, json, sys
from pathlib import Path
run_dir, gate_reason, nonce, elf_sha, verdict_path = sys.argv[1:6]
sys.path.insert(0, str(Path(verdict_path).parent))
from xw3_verdict import read_staged_summary, apply_gate_result, atomic_publish

staging_path = Path(run_dir) / "summary.case.json"
final_path = Path(run_dir) / "summary.json"
result_path = Path(run_dir) / "run-result.json"

summary = read_staged_summary(staging_path)
summary = apply_gate_result(summary, gate_reason)

reasons = list(summary.get("reasons", []))
summary["run_nonce"] = nonce
summary["elf_sha256"] = elf_sha
case_bindings = {}
cases_dir = Path(run_dir) / "cases"
if cases_dir.is_dir():
    for case_dir in sorted(path for path in cases_dir.iterdir() if path.is_dir()):
        capture = case_dir / "capture.bin"
        verdict = case_dir / "verdict.json"
        if capture.is_file() and verdict.is_file():
            case_bindings[case_dir.name] = {
                "capture_sha256": hashlib.sha256(capture.read_bytes()).hexdigest(),
                "capture_bytes": capture.stat().st_size,
                "verdict_sha256": hashlib.sha256(verdict.read_bytes()).hexdigest(),
            }
summary["cases"] = case_bindings

atomic_publish(final_path, summary)

result = {
    "result": summary["result"],
    "reasons": reasons,
    "run_nonce": nonce,
    "elf_sha256": elf_sha,
    "summary_sha256": hashlib.sha256(final_path.read_bytes()).hexdigest(),
}
result_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"[xw3] run-result: {result['result']} (summary sha256 {result['summary_sha256']})")
PYGATE

  if [[ -n "${GATE_REASON}" ]]; then
    echo "[xw3] RESULT: FAIL (${GATE_REASON})" >&2
    exit 1
  fi
  echo "[xw3] RESULT: PASS (fixture case ${FIXTURE_CASE})"
  exit 0
fi

# --------------------------------------------------------------------------
# real case plan
# --------------------------------------------------------------------------
if [[ -n "${ONLY_CASE}" ]]; then
  case "${ONLY_CASE}" in
    XW3-A1) run_real_case XW3-A1 golden accept ;;
    XW3-R1) run_real_case XW3-R1 corrupt_crc reject ;;
    XW3-O1) run_real_case XW3-O1 duplicate observe ;;
    XW3-O2) run_real_case XW3-O2 split observe ;;
    *) echo "[xw3] unknown case ${ONLY_CASE}" >&2; exit 2 ;;
  esac
  exit 0
fi

FAILED=0
if [[ "${XW3_SKIP_CASES:-0}" != "1" ]]; then
  run_real_case XW3-A1 golden accept || FAILED=1
  run_real_case XW3-R1 corrupt_crc reject || FAILED=1
  run_real_case XW3-O1 duplicate observe || FAILED=1
  run_real_case XW3-O2 split observe || FAILED=1
fi

# --------------------------------------------------------------------------
# fatal-before-PASS control (real): the no-wrap image panics at watchdog init;
# it must be rejected by the same verdict, and it is never a pass.
# --------------------------------------------------------------------------
CTRL_DIR="${RUN_DIR}/cases/XW3-C1"
mkdir -p "${CTRL_DIR}"
if [[ -f "${CONTROL_ELF}" ]]; then
  echo "[xw3] XW3-C1: fatal-before-PASS control (no-wrap image)"
  CTRL_PLAN="$(plan_for golden)"
  CTRL_RC="$(run_qemu "${CONTROL_ELF}" "${CTRL_DIR}/capture.bin" "${CTRL_PLAN}")"
  echo "[xw3] XW3-C1: qemu exit ${CTRL_RC}"
  set +e
  verify_case "${CTRL_DIR}" XW3-C1 golden accept "${CTRL_RC}" > "${CTRL_DIR}/control-verify.log" 2>&1
  CTRL_VERDICT_RC="$?"
  set -e
  {
    printf 'qemu_exit=%s\nverdict_exit=%s\n' "${CTRL_RC}" "${CTRL_VERDICT_RC}"
    grep -a "watchdog: init failed" "${CTRL_DIR}/capture.bin" >/dev/null \
      && echo "fatal_marker=watchdog: init failed" || echo "fatal_marker=ABSENT"
  } > "${CTRL_DIR}/control-result.txt"
  if [[ "${CTRL_VERDICT_RC}" -eq 0 ]]; then
    echo "[xw3] FAIL: the fatal control was ACCEPTED by the verdict (must be rejected)" >&2
    FAILED=1
  fi
  if ! grep -aq "watchdog: init failed" "${CTRL_DIR}/capture.bin"; then
    echo "[xw3] FAIL: the fatal control did not reproduce the expected panic" >&2
    FAILED=1
  fi
else
  echo "[xw3] MISSING PREREQUISITE: ${CONTROL_ELF} (fatal-before-PASS control)" >&2
  FAILED=1
fi

# --------------------------------------------------------------------------
# wrapper segregation evidence (measured and gated)
# --------------------------------------------------------------------------
if ! docker run --rm --pull=never --network=none --user "$(id -u):$(id -g)" \
  -v "${ROOT}:/workspace" -w /workspace "${IMG}" \
  bash -lc "
    echo '=== XW image symbols ==='
    riscv-rtems6-nm build/xw3-qemu/${ELF_NAME} | grep -E 'ff_pal_wd_hw|POSIX_Init'
    echo '=== XW image call sites ==='
    echo -n 'wrapper call sites (must be 3): '; riscv-rtems6-objdump -d build/xw3-qemu/${ELF_NAME} | grep -cE '(jal|j|jalr).*<__wrap_ff_pal_wd_hw' || true
    echo -n 'real-hook call sites (must be 0): '; riscv-rtems6-objdump -d build/xw3-qemu/${ELF_NAME} | grep -cE '(jal|j|jalr).*<ff_pal_wd_hw' || true
    echo '=== flight-shaped image from the same build ==='
    riscv-rtems6-nm build/xw3-qemu/frick_fsw | grep -E 'ff_pal_wd_hw|POSIX_Init'
    echo -n 'wrappers defined (must be 0): '; riscv-rtems6-nm build/xw3-qemu/frick_fsw | grep -c __wrap_ff_pal_wd_hw || true
    echo -n 'real-hook call sites (must be 3): '; riscv-rtems6-objdump -d build/xw3-qemu/frick_fsw | grep -cE '(jal|j|jalr).*<ff_pal_wd_hw' || true
  " > "${RUN_DIR}/nm-segregation.txt" 2>&1; then
  echo "[xw3] FAIL: the wrapper-segregation measurement did not run" >&2
  FAILED=1
fi
echo "[xw3] wrapper segregation written to ${RUN_DIR}/nm-segregation.txt"

# Gate the segregation counts (this was a recorded-only measurement in XW-2):
# the wrappers must be live only in the test-only image, and the flight-shaped
# image from the same build must call the real hooks.
SEG_OK=1
grep -q "wrapper call sites (must be 3): 3$" "${RUN_DIR}/nm-segregation.txt" || SEG_OK=0
grep -q "real-hook call sites (must be 0): 0$" "${RUN_DIR}/nm-segregation.txt" || SEG_OK=0
grep -q "wrappers defined (must be 0): 0$" "${RUN_DIR}/nm-segregation.txt" || SEG_OK=0
grep -q "real-hook call sites (must be 3): 3$" "${RUN_DIR}/nm-segregation.txt" || SEG_OK=0
if [[ "${SEG_OK}" -ne 1 ]]; then
  echo "[xw3] FAIL: wrapper segregation does not match the expected shape (see ${RUN_DIR}/nm-segregation.txt)" >&2
  FAILED=1
fi

# identity re-check after the run: the image must not have changed under us
ELF_SHA_AFTER="$(sha256sum "${ELF}" | cut -d' ' -f1)"
if [[ "${ELF_SHA_AFTER}" != "${ELF_SHA}" ]]; then
  echo "[xw3] FAIL: the image changed during the run (${ELF_SHA} -> ${ELF_SHA_AFTER})" >&2
  FAILED=1
fi

# --------------------------------------------------------------------------
# whole-run case verdict. It is produced only AFTER every gate above, and the
# final result written to summary.json/run-result.json includes those gates, so
# an aborted or gate-failed run can never leave a PASS summary behind.
# --------------------------------------------------------------------------
# Stage 1: case-only summary (no gate metadata) -> summary.case.json
set +e
python3 "${VERDICT}" summary --run-dir "${RUN_DIR}" --manifest "${MANIFEST}" \
  --run-index "${RUN_DIR}/run-index.json" \
  --json "${RUN_DIR}/summary.case.json" | tee "${RUN_DIR}/summary.log"
SUMMARY_RC="${PIPESTATUS[0]}"
set -e
echo "[xw3] summary exit: ${SUMMARY_RC}"

GATE_REASON=""
if [[ "${FAILED}" -ne 0 ]]; then
  GATE_REASON="runner_gate_failed"
elif [[ "${SUMMARY_RC}" -ne 0 ]]; then
  GATE_REASON="case_verdict_failed"
fi
# Stage 2: PYGATE reads staged case summary, adds gate metadata, atomically publishes final summary.json
# Pass the verdict script path explicitly so the helper import works (stdin __file__ is not the script path).
python3 - "${RUN_DIR}" "${GATE_REASON}" "${NONCE}" "${ELF_SHA}" "${VERDICT}" <<'PYGATE'
import hashlib, json, sys
from pathlib import Path
run_dir, gate_reason, nonce, elf_sha, verdict_path = sys.argv[1:6]
# Import atomic_publish from the verdict module
sys.path.insert(0, str(Path(verdict_path).parent))
from xw3_verdict import read_staged_summary, apply_gate_result, atomic_publish

staging_path = Path(run_dir) / "summary.case.json"
final_path = Path(run_dir) / "summary.json"
result_path = Path(run_dir) / "run-result.json"

# Read the case-only summary (staging)
summary = read_staged_summary(staging_path)
summary = apply_gate_result(summary, gate_reason)

reasons = list(summary.get("reasons", []))
summary["run_nonce"] = nonce
summary["elf_sha256"] = elf_sha
# Bind every retained case artifact into the summary so a PASS summary is
# unique to this run and cannot survive a wholesale case substitution.
case_bindings = {}
cases_dir = Path(run_dir) / "cases"
if cases_dir.is_dir():
    for case_dir in sorted(path for path in cases_dir.iterdir() if path.is_dir()):
        capture = case_dir / "capture.bin"
        verdict = case_dir / "verdict.json"
        if capture.is_file() and verdict.is_file():
            case_bindings[case_dir.name] = {
                "capture_sha256": hashlib.sha256(capture.read_bytes()).hexdigest(),
                "capture_bytes": capture.stat().st_size,
                "verdict_sha256": hashlib.sha256(verdict.read_bytes()).hexdigest(),
            }
summary["cases"] = case_bindings

# Atomic publish to final summary.json
atomic_publish(final_path, summary)

result = {
    "result": summary["result"],
    "reasons": reasons,
    "run_nonce": nonce,
    "elf_sha256": elf_sha,
    "summary_sha256": hashlib.sha256(final_path.read_bytes()).hexdigest(),
}
result_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
print(f"[xw3] run-result: {result['result']} (summary sha256 {result['summary_sha256']})")
PYGATE

if [[ -n "${GATE_REASON}" ]]; then
  echo "[xw3] RESULT: FAIL (${GATE_REASON})" >&2
  exit 1
fi

cat <<TXT
[xw3] PASS (real external TC on the serial input -> unchanged middleware ->
[xw3]       captured TM verified by the independent oracle; TEST-ONLY image).
[xw3] Not a device, not HIL, not N18, not all-I/O, no timing/WCET, no mission
[xw3] profile, no reset/independence claim; NC-PAL-01 / NC-EXT-198 remain OPEN.
TXT
exit 0
