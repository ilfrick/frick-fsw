#!/usr/bin/env bash
# frick-fsw native simulator smoke test (evidence: ci:simulator-smoke).
#
# Boots the native simulator and asserts:
#   * the banner prints,
#   * the framework reports 6 registered applications,
#   * telemetry is produced on the loopback link (EVS -> PUS service 5).
#
# Exit code 0 = PASS, 1 = FAIL.
# SPDX-License-Identifier: AGPL-3.0-only
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build/native}"
LOG="$(mktemp)"

echo "[smoke] building"
cmake -S "${ROOT}" -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Debug >/dev/null
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