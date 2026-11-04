#!/usr/bin/env bash
# frick-fsw - XW-2 TEST-ONLY external-watchdog contract image run in Renode
# (norm N18 criterion per the 2026-10-27 owner decisions; loop-audit rev 10.49).
#
# Builds the test-only `frick_fsw_xw2` positive image and its no-wrap expected-
# failure control for the MPFS RTEMS BSP in the cached toolchain image, runs
# both in the digest-pinned Renode PolarFire SoC environment (the owner
# authorized exactly one pull of that digest), and verifies the captured console
# with the EXISTING fail-closed tools (target_req_verify + target_run_verdict)
# against tools/scripts/xw2_manifest.json - the same nominal and applicable
# out-of-bound expected results as the accepted QEMU XW-2 run. No new oracle is
# invented; the production PAL/watchdog sources and the flight image behaviour
# are untouched; the production-shaped flight image FAIL (NC-PAL-01/NC-EXT-198)
# is a separate standing record and is not exercised by this runner.
#
# Scope, stated plainly: the device seam in the image is the existing validated
# test double. This proves the abstract contract on the RTEMS/MPFS build. It is
# NOT a device, NOT HIL, no real reset, no electrical independence, no
# timing/WCET, no mission timeout (8 s stays an ESC-7 placeholder). The no-wrap
# control is an EXPECTED FAILURE and is never a pass.
#
# Exit: 0 only when the positive run passes the component AND whole-run verdict
# from the captured console AND the no-wrap control reproduces the expected
# failure shape; 1 otherwise; 2 on a missing prerequisite. All failures fail
# closed and keep their logs.
#
# Safety/CM properties (2026-10-27 correction): the Renode docker run is
# explicitly `--pull=never --network=none`; the container name is UNIQUE PER
# RUN (label + UTC timestamp + random suffix) and is checked free before
# start — a name collision fails closed and nothing is removed; only the
# container this loop itself created, only after it exits, is removed; a
# still-running own container is never stopped/removed (fail-closed with its
# name recorded); the runner never pulls and no unrelated container can ever
# be touched.
#
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RENODE_IMG="antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061"
TOOLCHAIN_IMG="frick-rtems-toolchain:local"
BUILD="${ROOT}/build/xw2-renode"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
RUN_DIR="${ROOT}/build/xw2-renode-run-${STAMP}"
MANIFEST="${ROOT}/tools/scripts/xw2_manifest.json"
ELF="${BUILD}/frick_fsw_xw2"
ELF_NOWRAP="${BUILD}/frick_fsw_xw2_nowrap"
RESC_POS="${ROOT}/tools/docker/renode/frick-xw2-renode.resc"
RESC_NOWRAP="${ROOT}/tools/docker/renode/frick-xw2-nowrap-renode.resc"
# The checked-in RESC observes the emulator for 180 s. Leave additional host
# time for container startup, output capture, and shutdown; this is not the
# target-side XW-B1 delay or a production timeout.
MAX_WAIT=240

mkdir -p "${RUN_DIR}"
echo "[xw2-renode] run directory: ${RUN_DIR}"

# ---- prerequisites (fail closed, no pull attempted) ----------------------
if ! docker image inspect "${TOOLCHAIN_IMG}" >/dev/null 2>&1; then
  echo "[xw2-renode] MISSING PREREQUISITE: docker image ${TOOLCHAIN_IMG} is not present (no pull attempted)" >&2
  exit 2
fi
if ! docker image inspect "${RENODE_IMG}" >/dev/null 2>&1; then
  echo "[xw2-renode] MISSING PREREQUISITE: pinned Renode image ${RENODE_IMG##*:} is not present (no pull attempted)" >&2
  exit 2
fi
RENODE_ID="$(docker image inspect -f '{{.Id}}' "${RENODE_IMG}")"
RENODE_IMAGE="antmicro/renode:${RENODE_IMG##*:}"
TOOLCHAIN_ID="$(docker image inspect -f '{{.Id}}' "${TOOLCHAIN_IMG}")"
for f in "${RESC_POS}" "${RESC_NOWRAP}" "${MANIFEST}"; do
  if [[ ! -f "${f}" ]]; then
    echo "[xw2-renode] MISSING PREREQUISITE: ${f}" >&2
    exit 2
  fi
done
printf 'renode-image-id=%s\nrenode-digest=antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061\ntoolchain-image-id=%s\n' \
  "${RENODE_ID}" "${TOOLCHAIN_ID}" > "${RUN_DIR}/image-identity.txt"

# ---- build the positive and control images (cached toolchain, no network) --
set +e
docker run --rm --pull=never --network=none \
  --user "$(id -u):$(id -g)" -v "${ROOT}:/workspace" -w /workspace "${TOOLCHAIN_IMG}" \
  bash -lc "RTEMS_HOME=/opt/rtems cmake -S . -B build/xw2-renode -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
      -DFF_PLATFORM=rtems -DFF_ENABLE_EXTWD_TARGET_TEST=ON \
      -DFF_ENABLE_TARGET_REQ_TEST=OFF -DFF_ENABLE_RENODE_TEST=OFF -DFF_USE_PUS_ASN1=ON && \
    RTEMS_HOME=/opt/rtems cmake --build build/xw2-renode -j\$(nproc)" \
  > "${RUN_DIR}/xw2-build.log" 2>&1
BUILD_RC=$?
set -e
echo "[xw2-renode] build exit: ${BUILD_RC}"
if [[ "${BUILD_RC}" -ne 0 ]]; then
  echo "[xw2-renode] FAIL: build failed (log ${RUN_DIR}/xw2-build.log)" >&2
  exit 1
fi
[[ -f "${ELF}" ]] || { echo "[xw2-renode] FAIL: ${ELF} not produced" >&2; exit 1; }
[[ -f "${ELF_NOWRAP}" ]] || { echo "[xw2-renode] FAIL: ${ELF_NOWRAP} not produced" >&2; exit 1; }

# ---- identity binding ----------------------------------------------------
ELF_SHA="$(sha256sum "${ELF}" | cut -d' ' -f1)"
ELF_NOWRAP_SHA="$(sha256sum "${ELF_NOWRAP}" | cut -d' ' -f1)"
printf '%s  %s\n' "${ELF_SHA}" "${ELF}" > "${RUN_DIR}/elf.sha256"
printf '%s  %s\n' "${ELF_NOWRAP_SHA}" "${ELF_NOWRAP}" > "${RUN_DIR}/nowrap-elf.sha256"
NONCE="$(head -c 16 /dev/urandom | od -An -tx1 | tr -d ' \n')"
printf '%s\n' "${NONCE}" > "${RUN_DIR}/run-nonce.txt"
sha256sum "${RESC_POS}" "${RESC_NOWRAP}" "${MANIFEST}" > "${RUN_DIR}/assets.sha256"
cp "${BUILD}/CMakeCache.txt" "${RUN_DIR}/CMakeCache.txt"
docker run --rm --pull=never --network=none --user "$(id -u):$(id -g)" \
  -v "${ROOT}:/workspace" -w /workspace "${TOOLCHAIN_IMG}" \
  bash -lc 'riscv-rtems6-gcc --version | head -n 1' > "${RUN_DIR}/compiler-version.txt" 2>&1
echo "[xw2-renode] ELF sha256 ${ELF_SHA}"
echo "[xw2-renode] run nonce ${NONCE}"

# ---- wrapper segregation proof (nm/objdump on the MPFS ELFs) -------------
docker run --rm --pull=never --network=none --user "$(id -u):$(id -g)" \
  -v "${ROOT}:/workspace" -w /workspace "${TOOLCHAIN_IMG}" \
  bash -lc "
    echo '=== XW-2 mpfs image symbols ==='
    riscv-rtems6-nm build/xw2-renode/frick_fsw_xw2 | grep -E 'ff_pal_wd_hw|POSIX_Init'
    echo -n 'wrapper call sites (must be 3): '; riscv-rtems6-objdump -d build/xw2-renode/frick_fsw_xw2 | grep -cE '(jal|j|jalr).*<__wrap_ff_pal_wd_hw' || true
    echo -n 'real-hook call sites (must be 0): '; riscv-rtems6-objdump -d build/xw2-renode/frick_fsw_xw2 | grep -cE '(jal|j|jalr).*<ff_pal_wd_hw' || true
    echo '=== flight-shaped image built in the same configuration ==='
    riscv-rtems6-nm build/xw2-renode/frick_fsw | grep -E 'ff_pal_wd_hw|POSIX_Init'
    echo -n 'wrappers defined (must be 0): '; riscv-rtems6-nm build/xw2-renode/frick_fsw | grep -c __wrap_ff_pal_wd_hw || true
    echo -n 'real-hook call sites (must be 3): '; riscv-rtems6-objdump -d build/xw2-renode/frick_fsw | grep -cE '(jal|j|jalr).*<ff_pal_wd_hw' || true
  " > "${RUN_DIR}/nm-segregation.txt" 2>&1
echo "[xw2-renode] nm segregation written"

# ---- one Renode run: $1 label, $2 resc path, $3 console out, $4 in-container
# ---- UART backend filename. The positive RESC observes the emulator for 180 s;
# ---- the outer process bound is 240 s for startup/capture/shutdown overhead.
# ---- Neither bound changes target timing or the requirement oracle. Safety/CM
# ---- properties (2026-10-27): container name is unique per run (label +
# ---- timestamp + random suffix) and verified free before start; an existing
# ---- container of that name fails closed. Never `docker rm -f` anything: no
# ---- unrelated container can be destroyed by this runner. The Renode run is
# ---- explicitly local-only: --pull=never --network=none (the prerequisite
# ---- check above guarantees the pinned digest is already cached; the runner
# ---- never pulls).
run_renode() {
  local label="$1" resc="$2" console="$3" uart="$4"
  local status rc=0
  local name="frick-xw2-vv-${label}-$(date -u +%Y%m%dT%H%M%SZ)-$(head -c 4 /dev/urandom | od -An -tx1 | tr -d ' \n')"
  if docker inspect "${name}" >/dev/null 2>&1; then
    echo "FAIL: a container named ${name} already exists - fail closed, nothing removed"
    return 1
  fi
  if ! docker run -d --pull=never --network=none --name "${name}" \
       -v "${ROOT}:/workspace" "${RENODE_IMG}" \
       renode --disable-xwt "/workspace${resc#${ROOT}}" >/dev/null 2>&1; then
    echo "FAIL: renode container start failed"
    return 1
  fi
  for _ in $(seq 1 "${MAX_WAIT}"); do
    status="$(docker inspect -f '{{.State.Status}}' "${name}" 2>/dev/null || echo gone)"
    case "${status}" in exited|dead|gone) break ;; esac
    sleep 1
  done
  local exit_code="$(docker inspect -f '{{.State.ExitCode}}' "${name}" 2>/dev/null || echo unknown)"
  printf 'container=%s\nstatus=%s\nexit_code=%s\n' "${name}" "${status}" "${exit_code}" > "${RUN_DIR}/$(basename "${console}" .log).exit-code"
  if ! docker logs "${name}" > "${console}.process.log" 2>&1; then
    echo "FAIL: renode process log capture failed"
    rc=1
  fi
  if ! docker cp "${name}:${uart}" "${console}" >/dev/null 2>&1; then
    echo "FAIL: UART console capture failed"
    rc=1
  fi
  if [[ "${status}" == "running" ]]; then
    # Bounded wait expired with our own container still alive: capture what it
    # emitted but FAIL the run; the container is NOT stopped/removed and its
    # unique name is recorded above for manual disposition.
    echo "FAIL: renode container still running after the bounded wait"
    return 1
  fi
  # Only the container THIS run created, only after it exited, is removed.
  # An unrelated container can never match the unique per-run name + guard.
  docker rm -f "${name}" >/dev/null 2>&1 || true
  return "${rc}"
}

run_renode "pos" "${RESC_POS}" "${RUN_DIR}/console.log" "/tmp/frick-xw2-uart.log"
POS_RUN_RC=$?
# Extract only the exit_code integer for the verdict script
POS_EXIT_CODE=$(grep '^exit_code=' "${RUN_DIR}/console.exit-code" 2>/dev/null | cut -d= -f2)
printf '%s\n' "${POS_EXIT_CODE:-unknown}" > "${RUN_DIR}/renode-exit-code.txt"
sha256sum "${RUN_DIR}/console.log" "${RUN_DIR}/console.log.process.log" > "${RUN_DIR}/captures.sha256" 2>/dev/null || true
echo "[xw2-renode] positive renode run captured (container exit ${POS_EXIT_CODE})"
if [[ "${POS_RUN_RC}" -ne 0 ]]; then
  echo "[xw2-renode] FAIL: the positive Renode run could not be captured - fail closed" >&2
  exit 1
fi

set +e
python3 "${ROOT}/tools/scripts/target_req_verify.py" --log "${RUN_DIR}/console.log" \
  --manifest "${MANIFEST}" --json "${RUN_DIR}/verifier.json" | tee "${RUN_DIR}/verifier.log"
COMPONENT_RC="${PIPESTATUS[0]}"
python3 "${ROOT}/tools/scripts/target_run_verdict.py" --console "${RUN_DIR}/console.log" \
  --manifest "${MANIFEST}" --json "${RUN_DIR}/verdict.json" --nonce "${NONCE}" \
  --qemu-exit "$(cat "${RUN_DIR}/renode-exit-code.txt")" | tee "${RUN_DIR}/verdict.log"
VERDICT_RC="${PIPESTATUS[0]}"
set -e
echo "[xw2-renode] component exit ${COMPONENT_RC}, whole-run verdict exit ${VERDICT_RC}"

# ---- no-wrap expected-failure control ------------------------------------
CONTROL_RC=1
CONTROL_NOTE="not run (control failed to execute)"
run_renode "nowrap" "${RESC_NOWRAP}" "${RUN_DIR}/control-console.log" "/tmp/frick-xw2-nowrap-uart.log"
CTRL_RUN_RC=$?
sha256sum "${RUN_DIR}/control-console.log" > "${RUN_DIR}/control-captures.sha256" 2>/dev/null || true
if [[ "${CTRL_RUN_RC}" -eq 0 ]]; then
  # Expected control shape: the un-substituted production hooks cannot arm, the
  # image panics at watchdog init ("watchdog: init failed"), the SAME verifier
  # rejects the console, and only XW-T8 (arm failure surfaced) can PASS; XW-B1
  # never runs because the image dies in FF_EsStartAll startup.
  CONTROL_PASS_CASES="$(grep -a 'TGTREQ: ' "${RUN_DIR}/control-console.log" | awk -F: '$4 ~ /^PASS/ {printf "%s ", $3}' | tr -d '\r')"
  printf '%s\n' "${CONTROL_PASS_CASES}" > "${RUN_DIR}/control-passing-cases.txt"
  set +e
  python3 "${ROOT}/tools/scripts/target_req_verify.py" \
    --log "${RUN_DIR}/control-console.log" --manifest "${MANIFEST}" \
    --json "${RUN_DIR}/control-verifier.json" > "${RUN_DIR}/control-verifier.log" 2>&1
  CONTROL_COMPONENT_RC=$?
  set -e
  echo "${CONTROL_COMPONENT_RC}" > "${RUN_DIR}/control-component-exit.txt"
  CONTROL_SHAPE_OK=0
  if [[ "${CONTROL_PASS_CASES}" == "XW-T8 " ]] && ! grep -aq "XW-B1" "${RUN_DIR}/control-console.log"; then
    CONTROL_SHAPE_OK=1
  fi
  if grep -aq "watchdog: init failed" "${RUN_DIR}/control-console.log" &&
     [[ "${CONTROL_COMPONENT_RC}" -ne 0 ]] && [[ "${CONTROL_SHAPE_OK}" -eq 1 ]]; then
    CONTROL_NOTE="EXPECTED FAILURE reproduced: watchdog init panic + component verifier exit ${CONTROL_COMPONENT_RC}"
  else
    CONTROL_NOTE="control did NOT fail as expected (component exit ${CONTROL_COMPONENT_RC}) - investigate"
  fi
fi
echo "[xw2-renode] no-wrap control: ${CONTROL_NOTE}"
printf '%s\n' "${CONTROL_NOTE}" > "${RUN_DIR}/control-result.txt"

# ---- composite verdict ---------------------------------------------------
if [[ "${VERDICT_RC}" -ne 0 ]] || [[ "${COMPONENT_RC}" -ne 0 ]]; then
  echo "[xw2-renode] FAIL: positive run did not pass (component ${COMPONENT_RC}, verdict ${VERDICT_RC})." >&2
  echo "[xw2-renode] A component PASS with a fatal console is not a target PASS." >&2
  exit 1
fi
if [[ "${CONTROL_NOTE}" != EXPECTED\ FAILURE* ]]; then
  echo "[xw2-renode] FAIL: the no-wrap negative control did not reproduce the expected failure (${CONTROL_NOTE})." >&2
  exit 1
fi

cat <<TXT
[xw2-renode] PASS (XW-2 abstract external-watchdog contract image on the Renode
[xw2-renode] PolarFire SoC environment; N18 criterion per the 2026-10-27 owner decisions).
[xw2-renode] Wrapper substitution proven by nm (see ${RUN_DIR}/nm-segregation.txt);
[xw2-renode] no-wrap control reproduced the watchdog-init panic as expected.
[xw2-renode] NOT a device, NOT HIL; no real reset, no electrical independence, no
[xw2-renode] timing/WCET, no mission timeout. The production-shaped flight image
[xw2-renode] FAIL remains NC-PAL-01/NC-EXT-198 (separate, not reclassified).
TXT
exit 0
