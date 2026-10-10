#!/usr/bin/env bash
# frick-fsw - RTEMS 6.2 toolchain bootstrap (embedded brains ECSS pre-qualified
# baseline) for the Microchip RT PolarFire SoC (BSP riscv/mpfs64imafdc).
#
# Installs rtems-source-builder and builds RTEMS 6.2 + the BSP, so that
# `make rtems` produces the flight binary. Requires: git, gcc, make, python3.
#
# Usage:  ./tools/scripts/setup_rtems.sh [--jobs N]
# Result: $RTEMS_HOME/rtems/6/bin/riscv-rtems6-gcc (RTEMS_HOME defaults to
#         /opt/rtems; the CMake toolchain file reads the same variable)
#
# SPDX-License-Identifier: AGPL-3.0-only
set -euo pipefail

RTEMS_HOME="${RTEMS_HOME:-/opt/rtems}"
RTEMS_VERSION="6"
JOBS="${1:-$(nproc)}"
SRC="$HOME/rtems-source-builder"
RSB_SCRIPT="sb-set-builder"

echo "[setup_rtems] RTEMS_HOME=${RTEMS_HOME} jobs=${JOBS}"

sudo mkdir -p "${RTEMS_HOME}"
sudo chown -R "$(id -u):$(id -g)" "${RTEMS_HOME}"

if [[ ! -d "${RTEMS_HOME}/rtems/${RTEMS_VERSION}/bin" ]]; then
  if [[ ! -d "${SRC}" ]]; then
    echo "[setup_rtems] cloning rtems-source-builder"
    git clone --depth 1 --branch "6.2" \
      https://gitlab.rtems.org/rtems/tools/rtems-source-builder.git "${SRC}"
  fi

  echo "[setup_rtems] building toolchain + RTEMS ${RTEMS_VERSION} (this takes a while)"
  (
    cd "${SRC}/rtems"
    ../source-builder/sb-set-builder \
      --jobs="${JOBS}" \
      --prefix="${RTEMS_HOME}/rtems/${RTEMS_VERSION}" \
      6/rtems-riscv.bset
  )

  # Build the embedded brains ECSS pre-qualified baseline for the MPFS BSP.
  echo "[setup_rtems] building BSP riscv/mpfs64imafdc"
  (
    cd "${SRC}/rtems"
    ../source-builder/sb-set-builder \
      --jobs="${JOBS}" \
      --prefix="${RTEMS_HOME}/rtems/${RTEMS_VERSION}" \
      6/rtems-riscv.bset \
      --with-rtems-bsp=riscv/mpfs64imafdc
  )
fi

echo "[setup_rtems] toolchain check:"
"${RTEMS_HOME}/rtems/${RTEMS_VERSION}/bin/riscv-rtems6-gcc" --version
echo "[setup_rtems] DONE - run 'make rtems' to cross-compile frick-fsw"