#!/usr/bin/env bash
# frick-fsw - Renode functional validation (PolarFire SoC), norm N18.
#
# Runs the middleware under test on the emulated PolarFire SoC (Renode image,
# pinned) and checks for the "RENODE_TEST: PASS" marker.
#
# The Renode/docker process is run detached and killed after a bounded time, so
# the script can never hang.
#
# Evidence handling (NC-EXT-199): every run writes into a FRESH per-run
# directory under ${TMPDIR:-/tmp} and the verdict is taken ONLY from that
# directory. The historical fixed log paths (/tmp/frick-renode.log,
# /tmp/frick-console.log) are never read: they could belong to an earlier run
# (or to another user), and an unwritable path silently produced a false PASS
# from stale content. A missing, unreadable or empty capture now fails closed.
#
# Two logs are retained as evidence (norm N11): the UART console
# (build/renode/console.log) and the full Renode/docker process log
# (build/renode/renode-process.log). These copies are for retention and
# documentation only - the verdict never depends on them.
#
# The RTEMS image is built with `make renode-image` (FF_ENABLE_RENODE_TEST=ON).
# When it is absent the run is a recorded SKIP proxy (exit 0); when it is present
# the gate is in force: a missing marker fails (exit 1). See
# docs/vv/renode-environment.md.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061"
ELF="${ROOT}/build/renode/frick_fsw_renode.elf"
NAME="frick-renode-vv"
VERDICT="${ROOT}/tools/scripts/renode_vv_verdict.sh"

if ! docker image inspect "${IMG}" > /dev/null 2>&1; then
  docker pull "${IMG}" > /dev/null 2>&1 || { echo "[renode] SKIP: image unavailable"; exit 0; }
fi
if [[ ! -f "${ELF}" ]]; then
  echo "[renode] SKIP: target ELF absent (${ELF}); build with 'make renode-image'"
  exit 0
fi

# ---- fresh per-run directory -------------------------------------------
RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/frick-renode-vv.XXXXXX")"
if [[ -z "${RUN_DIR}" ]] || [[ ! -d "${RUN_DIR}" ]]; then
  echo "[renode] FAIL: could not create a fresh per-run log directory - fail closed"
  exit 1
fi
LOG="${RUN_DIR}/renode-process.log"
CONSOLE="${RUN_DIR}/uart-console.log"
STATUS="${RUN_DIR}/status.env"

cleanup() {
  docker rm -f "${NAME}" > /dev/null 2>&1 || true
}
trap cleanup EXIT

mkdir -p "${ROOT}/build/renode"

# ---- start the emulated target ----------------------------------------
docker rm -f "${NAME}" > /dev/null 2>&1 || true
if ! docker run -d --name "${NAME}" -v "${ROOT}:/workspace" "${IMG}" \
     renode --disable-xwt /workspace/tools/docker/renode/frick-renode.resc > /dev/null 2>&1; then
  echo "[renode] FAIL: the emulated target could not be started - fail closed"
  exit 1
fi

# ---- bounded wait; the container status/exit code are kept explicitly ----
WAIT_RESULT="running"
CONTAINER_STATUS="unknown"
CONTAINER_EXIT="unknown"
for _ in $(seq 1 120); do
  status="$(docker inspect -f '{{.State.Status}}' "${NAME}" 2>/dev/null || echo gone)"
  case "${status}" in
    exited|dead|gone) WAIT_RESULT="exited"; CONTAINER_STATUS="${status}"; break ;;
  esac
  sleep 1
done
if [[ "${WAIT_RESULT}" = "exited" ]]; then
  CONTAINER_EXIT="$(docker inspect -f '{{.State.ExitCode}}' "${NAME}" 2>/dev/null || echo unknown)"
else
  CONTAINER_STATUS="running"
fi
{
  echo "CONTAINER_STATUS=${CONTAINER_STATUS}"
  echo "CONTAINER_EXIT=${CONTAINER_EXIT}"
  echo "WAIT_RESULT=${WAIT_RESULT}"
} > "${STATUS}"

# ---- capture this run's own logs (fail closed on capture failure) -------
if ! docker logs "${NAME}" > "${LOG}" 2>"${RUN_DIR}/docker-logs.stderr"; then
  echo "[renode] FAIL: the Renode process log could not be captured - fail closed"
  exit 1
fi
if ! docker cp "${NAME}:/tmp/frick-uart.log" "${CONSOLE}" > /dev/null 2>&1; then
  echo "[renode] FAIL: the UART console could not be captured - fail closed"
  exit 1
fi

# ---- evidence retention copies (not used for the verdict) ---------------
# The copy is atomic (write beside the target, then rename) so that a failed
# copy can never leave a STALE file that looks like this run's evidence.
retain() {
    local src="$1" dst="$2"
    if ! cp "${src}" "${dst}.new" 2>/dev/null; then
        rm -f "${dst}" 2>/dev/null || true
        echo "[renode] WARNING: could not retain ${dst} (the previous copy was removed so it cannot be mistaken for this run)" >&2
        return 1
    fi
    if ! mv "${dst}.new" "${dst}" 2>/dev/null; then
        rm -f "${dst}.new" 2>/dev/null || true
        rm -f "${dst}" 2>/dev/null || true
        echo "[renode] WARNING: could not retain ${dst} (the previous copy was removed so it cannot be mistaken for this run)" >&2
        return 1
    fi
    return 0
}
retain "${LOG}" "${ROOT}/build/renode/renode-process.log"
retain "${CONSOLE}" "${ROOT}/build/renode/console.log"

# ---- verdict ------------------------------------------------------------
bash "${VERDICT}" "${RUN_DIR}"
rc=$?
echo "[renode] run artifacts: ${RUN_DIR}"
exit "${rc}"
