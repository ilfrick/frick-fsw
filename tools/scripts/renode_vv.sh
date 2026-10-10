#!/usr/bin/env bash
# frick-fsw - Renode functional validation (PolarFire SoC), norm N18.
#
# Runs the middleware under test on the emulated PolarFire SoC (Renode image,
# pinned) and checks for the "RENODE_TEST: PASS" marker.
#
# The Renode/docker process is run detached and killed after a bounded time, so
# the script can never hang.
#
# PREREQUISITES to be blocking: the RTEMS toolchain (`make renode-image` builds
# `build/renode/frick_fsw_renode.elf` with FF_ENABLE_RENODE_TEST=ON). Until then
# this is an advisory proxy (SKIP, exit 0) so it does not block changes while the
# environment is being brought up; see docs/vv/renode-environment.md.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061"
RESC="${ROOT}/tools/docker/renode/frick-renode.resc"
ELF="${ROOT}/build/renode/frick_fsw_renode.elf"
NAME="frick-renode-vv"
LOG="/tmp/frick-renode.log"

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
  renode --disable-xwt "${RESC}" > /dev/null 2>&1

for _ in $(seq 1 120); do
  running="$(docker inspect -f '{{.State.Running}}' "${NAME}" 2>/dev/null || echo false)"
  [[ "${running}" == "true" ]] || break
  sleep 1
done

docker logs "${NAME}" > "${LOG}" 2>&1 || true
cat "${ROOT}/build/renode/uart0.log" >> "${LOG}" 2>/dev/null || true
docker rm -f "${NAME}" > /dev/null 2>&1 || true

if grep -q "RENODE_TEST: PASS" "${LOG}"; then
  echo "[renode] PASS (RENODE_TEST: PASS)"
  exit 0
fi
# The ELF is present (checked above) but the marker is missing: a real functional
# failure, not a skip.
echo "[renode] FAIL: target ELF present but marker absent (see ${LOG})"
exit 1
