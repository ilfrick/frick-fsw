#!/usr/bin/env bash
# frick-fsw RTEMS cross build (CI evidence: ci:rtems-cross-build).
#
# If the RTEMS 6.2 toolchain (BSP riscv/mpfs64imafdc) is present under
# $RTEMS_HOME, performs the real cross build and FAILS (non-zero) on any build
# error. If the toolchain is absent it is a documented SKIP proxy (exit 0); the
# scheduled/unattended workflow installs the toolchain and performs the real
# build (see .github/workflows/nightly.yml, tools/scripts/setup_rtems.sh).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RTEMS_HOME="${RTEMS_HOME:-/opt/rtems}"
GCC="${RTEMS_HOME}/6/bin/riscv-rtems6-gcc"

if [[ ! -x "${GCC}" ]]; then
  echo "[rtems-cross] SKIP (toolchain absent): run tools/scripts/setup_rtems.sh or the scheduled workflow"
  echo "[rtems-cross] evidence for SSS-REQ-001 / SRS-REQ-037:"
  echo "  - host proxy: sources compile with -Wall -Wextra in ci-host-build"
  echo "  - full target build: scheduled workflow / local setup_rtems.sh"
  exit 0
fi

echo "[rtems-cross] toolchain found: ${GCC}"
cmake -S "${ROOT}" -B "${ROOT}/build/rtems" \
  -DCMAKE_TOOLCHAIN_FILE="${ROOT}/cmake/toolchains/rtems-riscv-mpfs.cmake" \
  -DFF_PLATFORM=rtems
cmake --build "${ROOT}/build/rtems" -j"$(nproc)"
echo "[rtems-cross] cross build: PASS"
