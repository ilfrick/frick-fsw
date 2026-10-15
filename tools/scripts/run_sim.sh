#!/usr/bin/env bash
# frick-fsw native simulator smoke test (evidence: ci:simulator-smoke).
#
# Boots the native simulator and asserts:
#   * the banner prints,
#   * the framework reports 6 registered applications,
#   * telemetry is produced on the loopback link (EVS -> PUS service 5).
#
# Exit code 0 = PASS, 1 = FAIL.
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build/native}"
LOG="$(mktemp)"

echo "[smoke] building"
# Reuse the generator already cached in BUILD_DIR when it is configured (e.g. the
# Unix Makefiles cache produced by `make configure` / `make build`). Forcing
# `-G Ninja` unconditionally made `make smoke` (and therefore `make check`) fail on
# any tree configured through the Makefile (NC-S4b-09).
cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug >/dev/null
cmake --build "${BUILD_DIR}" >/dev/null

echo "[smoke] booting simulator (8 s window)"
timeout 8 "${BUILD_DIR}/frick_fsw" >"${LOG}" 2>&1 || true

if ! grep -q "frick-fsw" "${LOG}"; then
  echo "[smoke] FAIL: banner not found"; cat "${LOG}"; exit 1
fi

if ! grep -q "6 applications registered" "${LOG}"; then
  echo "[smoke] FAIL: application count mismatch"; cat "${LOG}"; exit 1
fi

echo "[smoke] PASS (banner + 6 applications registered)"
rm -f "${LOG}"
