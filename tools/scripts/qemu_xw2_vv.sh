#!/usr/bin/env bash
# frick-fsw - XW-2 test-only emulated-target run (external-watchdog contract)
#
# Builds the TEST-ONLY `frick_fsw_xw2` image, runs XW-T1..XW-T12 + XW-B1 on QEMU
# `virt`, and verifies the console with the EXISTING fail-closed tools
# (target_req_verify + target_run_verdict) against tools/scripts/xw2_manifest.json.
#
# Two explicitly identified backends (recorded in the run directory):
#   docker: the cached `frick-rtems-toolchain:local` image, offline, no pull;
#   host:   an RTEMS 6.2 toolchain under $RTEMS_HOME plus qemu-system-riscv64
#           (the scheduled GitHub workflow installs both).
# The backend does not change the commands or the oracles; it is recorded so the
# evidence is attributable.
#
# Recorded knobs (both written into the run directory):
#   XW2_USE_PUS_ASN1  FF_USE_PUS_ASN1 for both images (default ON). This image
#                     contains no PUS codec, so OFF is contract-equivalent and
#                     drops the `make pus-asn1`/docker prerequisite that a
#                     pre-seeded CI container cannot satisfy (PA-03).
#   XW2_QEMU_TIMEOUT  per-image QEMU wall-clock budget in seconds (default 25).
#
# It also runs the NO-WRAP expected-failure control: the same sources built without
# the -Wl,--wrap substitution must abort at watchdog init. That control is recorded
# as an EXPECTED FAILURE; it is never a pass, and it is not a substitute for the
# positive run.
#
# Scope, stated plainly: the device seam is substituted by a validated test double.
# This proves the abstract contract on the RTEMS build - NOT a device, NOT HIL,
# NOT N18, no real reset, no electrical independence, no timing/WCET, no mission
# timeout (8 s stays an ESC-7 placeholder).
#
# Exit: 0 only when the positive run is component-PASS, whole-run PASS and the
# no-wrap control fails as expected; 1 otherwise; 2 on a missing prerequisite.
#
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="frick-rtems-toolchain:local"
BUILD="${ROOT}/build/xw2-qemu"
BUILD_NOWRAP="${ROOT}/build/xw2-qemu-nowrap"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
RUN_DIR="${ROOT}/build/xw2-run-${STAMP}"
MANIFEST="${ROOT}/tools/scripts/xw2_manifest.json"
ELF="${BUILD}/frick_fsw_xw2"
ELF_NOWRAP="${BUILD_NOWRAP}/frick_fsw_xw2_nowrap"

# PUS-C ASN.1 backend selection. The XW-2 image contains no PUS codec, so
# `OFF` is contract-equivalent for this gate and removes the `docker` dependency
# that `make pus-asn1` would introduce inside a pre-seeded CI container (PA-03).
# The effective value is recorded in the run directory.
PUS_ASN1="${XW2_USE_PUS_ASN1:-ON}"
# QEMU wall-clock budget per image. The positive run ends by exiting; a timeout
# is an accepted outcome only because the verdict is computed from the console
# by the fail-closed verifier (PA-10).
QEMU_TIMEOUT="${XW2_QEMU_TIMEOUT:-25}"

mkdir -p "${RUN_DIR}"
echo "[xw2] run directory: ${RUN_DIR}"

# ---- execution backend --------------------------------------------------
# Two interchangeable, explicitly identified backends:
#   docker: the cached `frick-rtems-toolchain:local` image (offline, no pull);
#   host:   an RTEMS 6.2 toolchain installed under $RTEMS_HOME (the scheduled
#           GitHub workflow installs it with tools/scripts/setup_rtems.sh).
# Both run the same commands and the same QEMU invocation; the backend is
# recorded in the run directory so the evidence is attributable.
RTEMS_HOME="${RTEMS_HOME:-/opt/rtems}"
if docker image inspect "${IMG}" >/dev/null 2>&1; then
  BACKEND="docker"
  IMAGE_ID="$(docker image inspect -f '{{.Id}}' "${IMG}")"
  echo "[xw2] backend docker, toolchain image ${IMG} id ${IMAGE_ID}"
  toolchain() {
    docker run --rm --pull=never --network=none \
      --user "$(id -u):$(id -g)" -v "${ROOT}:/workspace" -w /workspace "${IMG}" \
      bash -lc "$1"
  }
elif [[ -x "${RTEMS_HOME}/6/bin/riscv-rtems6-gcc" ]] &&
     command -v qemu-system-riscv64 >/dev/null 2>&1; then
  BACKEND="host"
  IMAGE_ID="host:${RTEMS_HOME}/6/bin/riscv-rtems6-gcc"
  echo "[xw2] backend host, toolchain ${IMAGE_ID}"
  toolchain() {
    ( cd "${ROOT}" && PATH="${RTEMS_HOME}/6/bin:${PATH}" bash -lc "$1" )
  }
else
  echo "[xw2] MISSING PREREQUISITE: neither docker image ${IMG} nor a host" >&2
  echo "[xw2] RTEMS 6.2 toolchain + qemu-system-riscv64 is available (no pull attempted)" >&2
  exit 2
fi
printf '%s\n' "${BACKEND}" > "${RUN_DIR}/backend.txt"
printf '%s\n' "${IMAGE_ID}" > "${RUN_DIR}/toolchain-id.txt"
printf '%s\n' "${PUS_ASN1}" > "${RUN_DIR}/pus-asn1.txt"
printf '%s\n' "${QEMU_TIMEOUT}" > "${RUN_DIR}/qemu-timeout.txt"
echo "[xw2] FF_USE_PUS_ASN1=${PUS_ASN1}, qemu timeout ${QEMU_TIMEOUT}s"

# ---- build the positive (wrapped) and control (no-wrap) images -----------
set +e
toolchain "cmake -S . -B build/xw2-qemu -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
      -DFF_PLATFORM=rtems-generic -DFF_ENABLE_EXTWD_TARGET_TEST=ON \
      -DFF_ENABLE_TARGET_REQ_TEST=OFF -DFF_ENABLE_RENODE_TEST=OFF -DFF_USE_PUS_ASN1=${PUS_ASN1} && \
    cmake --build build/xw2-qemu -j\$(nproc)" 2>&1 | tee "${RUN_DIR}/xw2-build.log"
BUILD_RC="${PIPESTATUS[0]}"
set -e
echo "[xw2] positive build exit: ${BUILD_RC}"
if [[ "${BUILD_RC}" -ne 0 ]]; then
  echo "[xw2] FAIL: build failed (log ${RUN_DIR}/xw2-build.log)" >&2
  exit 1
fi

set +e
toolchain "cmake -S . -B build/xw2-qemu-nowrap -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
      -DFF_PLATFORM=rtems-generic -DFF_ENABLE_EXTWD_TARGET_TEST=ON \
      -DFF_ENABLE_TARGET_REQ_TEST=OFF -DFF_ENABLE_RENODE_TEST=OFF -DFF_USE_PUS_ASN1=${PUS_ASN1} \
      -DFF_XW2_NOWRAP_CONTROL=ON && \
    cmake --build build/xw2-qemu-nowrap -j\$(nproc)" 2>&1 | tee "${RUN_DIR}/xw2-nowrap-build.log"
NW_BUILD_RC="${PIPESTATUS[0]}"
set -e
echo "[xw2] control build exit: ${NW_BUILD_RC}"
if [[ "${NW_BUILD_RC}" -ne 0 ]]; then
  echo "[xw2] FAIL: the no-wrap control image did not build (exit ${NW_BUILD_RC}); the control cannot be claimed." >&2
  exit 1
fi

[[ -f "${BUILD}/CMakeCache.txt" ]] && cp "${BUILD}/CMakeCache.txt" "${RUN_DIR}/CMakeCache.txt"
if [[ ! -f "${ELF}" ]]; then
  echo "[xw2] MISSING PREREQUISITE: ${ELF} was not produced" >&2
  exit 2
fi
if [[ ! -f "${MANIFEST}" ]]; then
  echo "[xw2] MISSING PREREQUISITE: ${MANIFEST}" >&2
  exit 2
fi

# ---- identity binding ---------------------------------------------------
ELF_SHA="$(sha256sum "${ELF}" | cut -d' ' -f1)"
printf '%s  %s\n' "${ELF_SHA}" "${ELF}" > "${RUN_DIR}/elf.sha256"
NONCE="$(head -c 16 /dev/urandom | od -An -tx1 | tr -d ' \n')"
printf '%s\n' "${NONCE}" > "${RUN_DIR}/run-nonce.txt"
toolchain 'riscv-rtems6-gcc --version | head -n 1' > "${RUN_DIR}/compiler-version.txt" 2>&1
COMPILER="$(cat "${RUN_DIR}/compiler-version.txt")"
echo "[xw2] ELF sha256 ${ELF_SHA}"
echo "[xw2] run nonce ${NONCE}"

# ---- wrapper segregation check (nm/objdump) ----------------------------
# The counts are ASSERTED, not merely printed: an image with 2 wrapper and 1
# real-hook call site is a mismatch and fails the run. Limit: this is a static
# symbol/call-site count on the linked image; it observes substitution, it does
# not prove dynamic behaviour (the no-wrap control provides the discrimination).
toolchain "
    echo '=== XW image symbols ==='
    riscv-rtems6-nm build/xw2-qemu/frick_fsw_xw2 | grep -E 'ff_pal_wd_hw|POSIX_Init'
    echo '=== XW image: call sites (wrappers vs real hooks) ==='
    echo XW2_WRAP_CALL_SITES=\$(riscv-rtems6-objdump -d build/xw2-qemu/frick_fsw_xw2 | grep -cE '(jal|j|jalr).*<__wrap_ff_pal_wd_hw' || true)
    echo XW2_REAL_CALL_SITES=\$(riscv-rtems6-objdump -d build/xw2-qemu/frick_fsw_xw2 | grep -cE '(jal|j|jalr).*<ff_pal_wd_hw' || true)
    echo '=== flight-shaped image built in the same configuration ==='
    riscv-rtems6-nm build/xw2-qemu/frick_fsw | grep -E 'ff_pal_wd_hw|POSIX_Init'
    echo FLIGHT_WRAP_DEFINED=\$(riscv-rtems6-nm build/xw2-qemu/frick_fsw | grep -c __wrap_ff_pal_wd_hw || true)
    echo FLIGHT_REAL_CALL_SITES=\$(riscv-rtems6-objdump -d build/xw2-qemu/frick_fsw | grep -cE '(jal|j|jalr).*<ff_pal_wd_hw' || true)
  " > "${RUN_DIR}/nm-segregation.txt" 2>&1
echo "[xw2] nm segregation written"

seg_count() {
  sed -n "s/^$1=//p" "${RUN_DIR}/nm-segregation.txt" | tr -d '\r' | head -n 1
}
SEG_FAIL=0
for expect in XW2_WRAP_CALL_SITES:3 XW2_REAL_CALL_SITES:0 \
              FLIGHT_WRAP_DEFINED:0 FLIGHT_REAL_CALL_SITES:3; do
  key="${expect%%:*}"
  want="${expect##*:}"
  got="$(seg_count "${key}")"
  if [[ "${got}" != "${want}" ]]; then
    echo "[xw2] SEGREGATION MISMATCH: ${key}=${got:-<absent>} (expected ${want})" >&2
    SEG_FAIL=1
  fi
done
printf '%s\n' "${SEG_FAIL}" > "${RUN_DIR}/nm-segregation.exit"
if [[ "${SEG_FAIL}" -ne 0 ]]; then
  echo "[xw2] FAIL: wrapper segregation counts do not match; the substitution claim is withdrawn." >&2
  exit 1
fi
echo "[xw2] wrapper segregation counts asserted: 3/0 (XW image) and 0/3 (flight image)"

# ---- positive run -------------------------------------------------------
run_qemu() { # $1 = elf path, $2 = console path
  local rc=0
  toolchain "timeout ${QEMU_TIMEOUT} qemu-system-riscv64 -M virt -nographic -bios ${1#${ROOT}/}" \
    > "${2}" 2>&1 || rc=$?
  echo "${rc}"
}

POS_START_NS="$(date -u +%s%N)"
QEMU_RC="$(run_qemu "${ELF}" "${RUN_DIR}/console.log")"
POS_ELAPSED_NS="$(( $(date -u +%s%N) - POS_START_NS ))"
printf '%s\n' "${POS_ELAPSED_NS}" > "${RUN_DIR}/positive-elapsed-ns.txt"
echo "[xw2] positive qemu exit: ${QEMU_RC} (0/124 accepted), elapsed ${POS_ELAPSED_NS} ns of ${QEMU_TIMEOUT}s budget"
printf '%s\n' "${QEMU_RC}" > "${RUN_DIR}/qemu-exit-code.txt"

set +e
python3 "${ROOT}/tools/scripts/target_req_verify.py" --log "${RUN_DIR}/console.log" \
  --manifest "${MANIFEST}" --json "${RUN_DIR}/verifier.json" | tee "${RUN_DIR}/verifier.log"
COMPONENT_RC="${PIPESTATUS[0]}"
python3 "${ROOT}/tools/scripts/target_run_verdict.py" --console "${RUN_DIR}/console.log" \
  --manifest "${MANIFEST}" --json "${RUN_DIR}/verdict.json" --nonce "${NONCE}" \
  --qemu-exit "${QEMU_RC}" | tee "${RUN_DIR}/verdict.log"
VERDICT_RC="${PIPESTATUS[0]}"
set -e
echo "[xw2] component exit ${COMPONENT_RC}, whole-run verdict exit ${VERDICT_RC}"

# ---- no-wrap expected-failure control ----------------------------------
CONTROL_RC=1
CONTROL_NOTE="not run (control image absent)"
if [[ -f "${ELF_NOWRAP}" ]]; then
  sha256sum "${ELF_NOWRAP}" > "${RUN_DIR}/control-elf.sha256"
  CONTROL_RC="$(run_qemu "${ELF_NOWRAP}" "${RUN_DIR}/control-console.log")"
  printf '%s\n' "${CONTROL_RC}" > "${RUN_DIR}/control-exit.txt"
  # The control must be rejected by the SAME verifier that accepts the wrapped
  # image, and must carry the watchdog panic. Note XW-T8 legitimately PASSES in
  # the control (it asserts that an arm failure is reported), so "no PASS at all"
  # is the wrong predicate - the manifest coverage is the predicate.
  set +e
  python3 "${ROOT}/tools/scripts/target_req_verify.py" \
    --log "${RUN_DIR}/control-console.log" --manifest "${MANIFEST}" \
    --json "${RUN_DIR}/control-verifier.json" > "${RUN_DIR}/control-verifier.log" 2>&1
  CONTROL_COMPONENT_RC=$?
  set -e
  echo "${CONTROL_COMPONENT_RC}" > "${RUN_DIR}/control-component-exit.txt"
  # Expected control shape: the harness runs but cannot arm, so XW-T8 (which
  # asserts that an arm failure is reported) is the only passing case, and the
  # post-boot case XW-B1 never runs because the image panics at FF_EsStartAll.
  CONTROL_PASS_CASES="$(grep -a 'TGTREQ: ' "${RUN_DIR}/control-console.log" | awk -F: '$4 ~ /^PASS/ {printf "%s ", $3}' | tr -d '\r')"
  printf '%s\n' "${CONTROL_PASS_CASES}" > "${RUN_DIR}/control-passing-cases.txt"
  CONTROL_SHAPE_OK=0
  if [[ "${CONTROL_PASS_CASES}" == "XW-T8 " ]] && ! grep -aq "XW-B1" "${RUN_DIR}/control-console.log"; then
    CONTROL_SHAPE_OK=1
  fi
  if grep -aq "watchdog: init failed" "${RUN_DIR}/control-console.log" &&
     [[ "${CONTROL_COMPONENT_RC}" -ne 0 ]] && [[ "${CONTROL_SHAPE_OK}" -eq 1 ]]; then
    CONTROL_NOTE="EXPECTED FAILURE reproduced: watchdog init panic + component verifier exit ${CONTROL_COMPONENT_RC}"
  else
    CONTROL_NOTE="control did NOT fail as expected (component exit ${CONTROL_COMPONENT_RC}) - investigate"
    CONTROL_RC=0   # force the overall failure below
  fi
fi
echo "[xw2] no-wrap control: ${CONTROL_NOTE}"
printf '%s\n' "${CONTROL_NOTE}" > "${RUN_DIR}/control-result.txt"

# ---- composite verdict -------------------------------------------------
if [[ "${VERDICT_RC}" -ne 0 ]]; then
  echo "[xw2] FAIL: positive run did not pass (component ${COMPONENT_RC}, verdict ${VERDICT_RC})." >&2
  echo "[xw2] A component PASS with a fatal console, or a failed QEMU process, is not a target PASS." >&2
  exit 1
fi
if [[ "${CONTROL_NOTE}" != EXPECTED\ FAILURE* ]]; then
  echo "[xw2] FAIL: the no-wrap negative control did not reproduce the expected failure (${CONTROL_NOTE})." >&2
  exit 1
fi

cat <<TXT
[xw2] PASS (abstract external-watchdog contract on a TEST-ONLY emulated-target image).
[xw2] Wrapper segregation counts ASSERTED from nm/objdump (see ${RUN_DIR}/nm-segregation.txt);
[xw2] no-wrap control reproduced the watchdog-init panic as expected. NOT a device, NOT HIL,
[xw2] NOT N18, no real reset, no electrical independence, no timing/WCET, no mission timeout.
TXT
exit 0
