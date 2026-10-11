#!/usr/bin/env bash
# frick-fsw - Renode V&V environment self-test (norm N18).
#
# Reproduces the "environment is ready" evidence: the pinned Renode image starts
# and loads the Microchip PolarFire SoC platform. The Renode/docker process is run
# detached and killed after a bounded time, so the script can never hang.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061"
NAME="frick-renode-env"
LOG="/tmp/frick-renode-env.log"

if ! docker image inspect "${IMG}" > /dev/null 2>&1; then
  docker pull "${IMG}" > /dev/null 2>&1 || { echo "[renode-env] SKIP: image unavailable"; exit 0; }
fi

docker rm -f "${NAME}" > /dev/null 2>&1 || true
docker run -d --name "${NAME}" -v "${ROOT}:/workspace" "${IMG}" \
  renode --disable-xwt /workspace/tools/docker/renode/renode_env.resc > /dev/null 2>&1

for _ in $(seq 1 60); do
  status="$(docker inspect -f '{{.State.Status}}' "${NAME}" 2>/dev/null || echo gone)"
  case "${status}" in exited|dead|gone) break ;; esac
  sleep 1
done

docker logs "${NAME}" > "${LOG}" 2>&1 || true
docker rm -f "${NAME}" > /dev/null 2>&1 || true

if grep -q "System bus created." "${LOG}" && ! grep -qiE "exception|unhandled|error" "${LOG}"; then
  echo "[renode-env] PASS: Renode runs and the PolarFire SoC board platform loads"
  exit 0
fi
echo "[renode-env] FAIL: platform did not load (see ${LOG})"
exit 1
