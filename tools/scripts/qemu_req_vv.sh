#!/usr/bin/env bash
# frick-fsw - T1 requirement-keyed target suite runner (generic RTEMS + QEMU)
#
# Builds the test-only requirement-test image offline in the cached toolchain
# image, runs it on QEMU `virt`, captures the console, and reports two
# independent results:
#
#   COMPONENT - the per-case evidence verdict (tools/scripts/target_req_verify.py)
#   WHOLE-RUN - the run verdict including panic/fatal markers
#               (tools/scripts/target_run_verdict.py)
#
# The runner's exit code is the WHOLE-RUN verdict: a console that panics is a
# FAILED run even when every case line passed and QEMU's process exit code is 0.
# That separation exists because the first T1 delivery reported a target PASS
# for an image that aborted at watchdog init (REVIEW-T1 F1).
#
# Scope of the evidence (stated, not implied): the generic `rv64imafdc` BSP has
# no MPFS peripherals, and the suite runs before `FF_EsStartAll()` / watchdog
# init in the test-only pre-production-init harness. This is logical-chain and
# boot evidence only: not MPFS peripheral evidence, not a flight-like
# functional boot, not norm N18, and no timing/WCET claim.
#
# Exit semantics (N10: a check that can fail; never 0 for "skipped"):
#   0  component PASS and no fatal marker (whole run PASS)
#   1  build failure, QEMU failure, component failure, or whole-run fatal
#   2  missing prerequisite (toolchain image, manifest or built ELF)
#
# Test seam (disclosed, N10-M3): T1_QEMU_CMD / T1_RUN_ROOT / T1_SKIP_BUILD /
# T1_ELF exist only for the offline fixtures, are refused unless
# T1_ALLOW_QEMU_STUB=1, and the QEMU command boots "${ELF#${ROOT}/}" - the same
# path whose SHA-256 is recorded - so the advertised elf_sha256 is the booted
# image.
#
# No network, no pulls, no installs: the container runs with --pull=never and
# --network=none, and the repository is the only mount (no $HOME, no device, no
# docker socket, no --privileged).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="frick-rtems-toolchain:local"
BUILD="${ROOT}/build/t1-qemu"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
ELF="${BUILD}/frick_fsw_reqtest"
MANIFEST="${ROOT}/tools/scripts/target_req_manifest.json"
# Test-only overrides for the fail-closed regression fixture. They are refused
# unless T1_ALLOW_QEMU_STUB=1 is set explicitly, so a normal invocation can never
# skip the build or substitute the emulator command by accident.
if [[ "${T1_ALLOW_QEMU_STUB:-0}" == "1" ]]; then
  QEMU_CMD="${T1_QEMU_CMD:-}"
  RUN_ROOT="${T1_RUN_ROOT:-${ROOT}/build}"
  SKIP_BUILD="${T1_SKIP_BUILD:-0}"
  if [[ -n "${T1_ELF:-}" ]]; then
    ELF="${T1_ELF}"   # test-only: path to the image under test
  fi
else
  if [[ -n "${T1_QEMU_CMD:-}" || -n "${T1_RUN_ROOT:-}" || -n "${T1_SKIP_BUILD:-}" || -n "${T1_ELF:-}" ]]; then
    echo "[t1] REFUSED: T1_QEMU_CMD/T1_RUN_ROOT/T1_SKIP_BUILD/T1_ELF require T1_ALLOW_QEMU_STUB=1 (test-only)" >&2
    exit 2
  fi
  QEMU_CMD=""
  RUN_ROOT="${ROOT}/build"
  SKIP_BUILD=0
fi
# A gated stub run is marked in the directory name so its artifacts can never be
# mistaken for a real campaign.
if [[ "${T1_ALLOW_QEMU_STUB:-0}" == "1" ]]; then RUN_PREFIX="-stub"; else RUN_PREFIX=""; fi
RUN_DIR="${RUN_ROOT}/t1-run${RUN_PREFIX}-${STAMP}"
BUILD_LOG="${RUN_DIR}/qemu-build.log"
CONSOLE="${RUN_DIR}/console.log"

mkdir -p "${RUN_DIR}"
echo "[t1] run directory: ${RUN_DIR}"

# ---- prerequisite: cached toolchain image -------------------------------
if [[ "${SKIP_BUILD}" == "1" ]]; then
  IMAGE_ID="(test-only stub run: no image)"
else
if ! docker image inspect "${IMG}" >/dev/null 2>&1; then
  echo "[t1] MISSING PREREQUISITE: docker image ${IMG} is not present (no pull is attempted)" >&2
  exit 2
fi
IMAGE_ID="$(docker image inspect -f '{{.Id}}' "${IMG}")"
echo "[t1] toolchain image ${IMG} id ${IMAGE_ID}"
fi

# ---- configure + build (offline, isolated output dir) -------------------
if [[ "${SKIP_BUILD}" == "1" ]]; then
  echo "[t1] TEST-ONLY: build skipped (T1_SKIP_BUILD=1); reusing ${ELF}"
else
set +e
docker run --rm --pull=never --network=none \
  --user "$(id -u):$(id -g)" \
  -v "${ROOT}:/workspace" -w /workspace \
  "${IMG}" \
  bash -lc "cmake -S . -B build/t1-qemu \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
      -DFF_PLATFORM=rtems-generic \
      -DFF_ENABLE_TARGET_REQ_TEST=ON \
      -DFF_ENABLE_RENODE_TEST=OFF \
      -DFF_USE_PUS_ASN1=ON && \
    cmake --build build/t1-qemu -j\$(nproc)" 2>&1 | tee "${BUILD_LOG}"

BUILD_RC="${PIPESTATUS[0]}"
set -e
echo "[t1] build exit: ${BUILD_RC}"
if [[ "${BUILD_RC}" -ne 0 ]]; then
  echo "[t1] FAIL: build failed (exit ${BUILD_RC}; log ${BUILD_LOG})" >&2
  exit 1
fi
[[ -f "${BUILD}/CMakeCache.txt" ]] && cp "${BUILD}/CMakeCache.txt" "${RUN_DIR}/CMakeCache.txt"
fi

if [[ ! -f "${ELF}" ]]; then
  echo "[t1] MISSING PREREQUISITE: ${ELF} was not produced" >&2
  exit 2
fi
if [[ ! -f "${MANIFEST}" ]]; then
  echo "[t1] MISSING PREREQUISITE: manifest ${MANIFEST}" >&2
  exit 2
fi

# ---- ELF identity binding (R5) -----------------------------------------
ELF_SHA="$(sha256sum "${ELF}" | cut -d' ' -f1)"
echo "[t1] ELF ${ELF}"
echo "[t1] ELF sha256 ${ELF_SHA}"
if [[ -f "${RUN_DIR}/elf.sha256" ]]; then
  RECORDED_SHA="$(cut -d' ' -f1 < "${RUN_DIR}/elf.sha256")"
  if [[ "${RECORDED_SHA}" != "${ELF_SHA}" ]]; then
    echo "[t1] FAIL: run directory ELF hash ${RECORDED_SHA} does not match the ELF executed (${ELF_SHA})" >&2
    exit 1
  fi
fi
printf '%s  %s\n' "${ELF_SHA}" "${ELF}" > "${RUN_DIR}/elf.sha256"
READBACK_SHA="$(cut -d' ' -f1 < "${RUN_DIR}/elf.sha256")"
if [[ "${READBACK_SHA}" != "${ELF_SHA}" ]]; then
  echo "[t1] FAIL: elf.sha256 read-back mismatch (${READBACK_SHA} != ${ELF_SHA})" >&2
  exit 1
fi

NONCE="$(head -c 16 /dev/urandom | od -An -tx1 | tr -d ' \n')"
printf '%s\n' "${NONCE}" > "${RUN_DIR}/run-nonce.txt"
echo "[t1] run nonce ${NONCE}"

if [[ "${SKIP_BUILD}" == "1" ]]; then
  echo "(test-only stub run: compiler probe skipped)" > "${RUN_DIR}/compiler-version.txt"
else
docker run --rm --pull=never --network=none \
  --user "$(id -u):$(id -g)" \
  -v "${ROOT}:/workspace" -w /workspace \
  "${IMG}" \
  bash -lc 'riscv-rtems6-gcc --version | head -n 1' > "${RUN_DIR}/compiler-version.txt" 2>&1
fi
COMPILER="$(cat "${RUN_DIR}/compiler-version.txt")"
echo "[t1] compiler: ${COMPILER}"

python3 - "${RUN_DIR}" "${ELF}" "${ELF_SHA}" "${NONCE}" "${MANIFEST}" "${IMAGE_ID}" "${COMPILER}" <<'PY'
import hashlib, json, os, sys, datetime
run_dir, elf, elf_sha, nonce, manifest, image_id, compiler = sys.argv[1:8]
meta = {
    "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds"),
    "elf": elf,
    "elf_sha256": elf_sha,
    "run_nonce": nonce,
    "manifest": manifest,
    "manifest_sha256": hashlib.sha256(open(manifest, "rb").read()).hexdigest(),
    "toolchain_image": os.environ.get("T1_IMAGE", "frick-rtems-toolchain:local"),
    "toolchain_image_id": image_id,
    "compiler": compiler,
    "whole_run_expected": "FAIL (watchdog init panic; the suite runs before FF_EsStartAll)",
}
with open(os.path.join(run_dir, "run-meta.json"), "w", encoding="utf-8") as handle:
    json.dump(meta, handle, indent=2, sort_keys=True)
    handle.write("\n")
print(f"[t1] run-meta.json written (nonce {nonce})")
PY

# ---- run on QEMU (bounded; the image idles or aborts after the suite) ---
QEMU_RC=0
if [[ -n "${QEMU_CMD}" ]]; then
  echo "[t1] TEST-ONLY: QEMU command overridden by T1_QEMU_CMD"
  bash -c "${QEMU_CMD}" > "${CONSOLE}" 2>&1 || QEMU_RC=$?
else
  docker run --rm --pull=never --network=none \
    --user "$(id -u):$(id -g)" \
    -v "${ROOT}:/workspace" -w /workspace \
    "${IMG}" \
    bash -lc "timeout 25 qemu-system-riscv64 -M virt -nographic -bios ${ELF#${ROOT}/}" \
    > "${CONSOLE}" 2>&1 || QEMU_RC=$?
fi
echo "[t1] qemu exit: ${QEMU_RC} (0 and 124 are the accepted codes; 124 = bounded timeout)"
printf '%s\n' "${QEMU_RC}" > "${RUN_DIR}/qemu-exit-code.txt"
if [[ "${QEMU_RC}" -ne 0 && "${QEMU_RC}" -ne 124 ]]; then
  echo "[t1] QEMU PROCESS FAILURE: exit ${QEMU_RC} (evidence still collected below)" >&2
fi

ELF_SHA_AFTER="$(sha256sum "${ELF}" | cut -d' ' -f1)"
if [[ "${ELF_SHA_AFTER}" != "${ELF_SHA}" ]]; then
  echo "[t1] FAIL: the image changed between hashing and the run (${ELF_SHA} -> ${ELF_SHA_AFTER})." >&2
  exit 1
fi

# ---- COMPONENT result (per-case evidence) -------------------------------
set +e
python3 "${ROOT}/tools/scripts/target_req_verify.py" \
  --log "${CONSOLE}" \
  --manifest "${MANIFEST}" \
  --json "${RUN_DIR}/verifier.json" | tee "${RUN_DIR}/verifier.log"
COMPONENT_RC="${PIPESTATUS[0]}"
set -e
echo "[t1] component verifier exit: ${COMPONENT_RC}"

# ---- WHOLE-RUN verdict (component + panic/fatal markers) ----------------
set +e
python3 "${ROOT}/tools/scripts/target_run_verdict.py" \
  --console "${CONSOLE}" \
  --manifest "${MANIFEST}" \
  --json "${RUN_DIR}/verdict.json" \
  --nonce "${NONCE}" \
  --qemu-exit "${QEMU_RC}" | tee "${RUN_DIR}/verdict.log"
VERDICT_RC="${PIPESTATUS[0]}"
set -e
echo "[t1] whole-run verdict exit: ${VERDICT_RC}"

if [[ "${VERDICT_RC}" -ne 0 ]]; then
  if [[ "${QEMU_RC}" -ne 0 && "${QEMU_RC}" -ne 124 ]]; then
    echo "[t1] FAIL: QEMU process failure (exit ${QEMU_RC}); console ${CONSOLE}, verdict ${RUN_DIR}/verdict.log." >&2
  fi
  echo "[t1] FAIL: whole target run did not pass (console ${CONSOLE}, verdict ${RUN_DIR}/verdict.log)." >&2
  echo "[t1] A component PASS with a panic/fatal console or a failed QEMU process is NOT a target PASS." >&2
  exit 1
fi

echo "[t1] PASS: component cases verified and no fatal marker (logical chain only; not N18, not MPFS peripherals)"
exit 0
