#!/usr/bin/env bash
# frick-fsw - Renode functional validation (PolarFire SoC), norm N18.
#
# Runs the middleware under test on the emulated PolarFire SoC (Renode image,
# pinned) and checks for the "RENODE_TEST: PASS" marker.
#
# The Renode/docker process is run detached and killed after a bounded time, so
# the script can never hang.
#
# Two logs are retained for evidence (norm N11): the UART console
# (build/renode/console.log) and the full Renode/docker process log
# (build/renode/renode-process.log), both best-effort host copies.
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
LOG="/tmp/frick-renode.log"
CONSOLE="/tmp/frick-console.log"

if ! docker image inspect "${IMG}" > /dev/null 2>&1; then
  docker pull "${IMG}" > /dev/null 2>&1 || { echo "[renode] SKIP: image unavailable"; exit 0; }
fi
if [[ ! -f "${ELF}" ]]; then
  echo "[renode] SKIP: target ELF absent (${ELF}); build with 'make renode-image'"
  exit 0
fi

docker rm -f "${NAME}" > /dev/null 2>&1 || true
mkdir -p "${ROOT}/build/renode"
docker run -d --name "${NAME}" -v "${ROOT}:/workspace" "${IMG}" \
  renode --disable-xwt /workspace/tools/docker/renode/frick-renode.resc > /dev/null 2>&1

for _ in $(seq 1 120); do
  status="$(docker inspect -f '{{.State.Status}}' "${NAME}" 2>/dev/null || echo gone)"
  case "${status}" in exited|dead|gone) break ;; esac
  sleep 1
done

docker logs "${NAME}" > "${LOG}" 2>&1 || true
# Copy the UART log out of the container to a host-writable path (a root-owned
# build/renode/ would otherwise make this fail silently and yield a false FAIL).
: > "${CONSOLE}"
docker cp "${NAME}:/tmp/frick-uart.log" "${CONSOLE}" > /dev/null 2>&1 || true
cat "${CONSOLE}" >> "${LOG}" 2>/dev/null || true
# Best-effort artifact copies for documentation/grepping convenience: the UART
# console and the full Renode process log (norm N11 evidence retention).
docker cp "${NAME}:/tmp/frick-uart.log" "${ROOT}/build/renode/console.log" > /dev/null 2>&1 || true
cp "${LOG}" "${ROOT}/build/renode/renode-process.log" > /dev/null 2>&1 || true
docker rm -f "${NAME}" > /dev/null 2>&1 || true

if grep -q "RENODE_TEST: PASS" "${LOG}"; then
  echo "[renode] PASS (RENODE_TEST: PASS)"
  exit 0
fi
# The ELF is present (checked above) but the marker is missing: a real functional
# failure, not a skip.
echo "[renode] FAIL: target ELF present but marker absent (see ${LOG})"
exit 1
