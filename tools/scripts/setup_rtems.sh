#!/usr/bin/env bash
# frick-fsw - RTEMS 6.2 toolchain bootstrap (open-source RTEMS project release)
# for the Microchip RT PolarFire SoC (BSP riscv/mpfs64imafdc).
#
# Uses only upstream/open RTEMS material: the RTEMS Source Builder and the RTEMS
# source from the RTEMS project (https://rtems.org, https://gitlab.rtems.org).
# No vendor/commercial baseline or pre-built toolchain is used.
#
# Installs rtems-source-builder, builds the riscv toolchain and the RTEMS 6.2
# kernel for the mpfs64imafdc BSP, so that `make rtems` produces the flight
# binary. Requires: git, gcc, make, python3 and the RSB prerequisites.
#
# Usage:  ./tools/scripts/setup_rtems.sh [jobs]
# Result: $RTEMS_HOME/6/bin/riscv-rtems6-gcc (RTEMS_HOME defaults to /opt/rtems,
#         so the toolchain is installed under /opt/rtems/6; the CMake toolchain
#         file reads the same variable)
#
# The same build is available containerised and reproducible via
# `make rtems-toolchain` (tools/docker/rtems/Dockerfile).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -euo pipefail

RTEMS_HOME="${RTEMS_HOME:-/opt/rtems}"
RTEMS_VERSION="6"
JOBS="${1:-$(nproc)}"
RSB_REF="6.2"
SRC="${RSB_SRC:-$HOME/rtems-source-builder}"
BSP="riscv/mpfs64imafdc"

echo "[setup_rtems] RTEMS_HOME=${RTEMS_HOME} jobs=${JOBS} rsb=${RSB_REF} bsp=${BSP}"

sudo mkdir -p "${RTEMS_HOME}"
sudo chown -R "$(id -u):$(id -g)" "${RTEMS_HOME}"

if [[ ! -d "${RTEMS_HOME}/${RTEMS_VERSION}/bin" ]]; then
  if [[ ! -d "${SRC}" ]]; then
    echo "[setup_rtems] cloning rtems-source-builder (open RTEMS project, ref ${RSB_REF})"
    git clone --depth 1 --branch "${RSB_REF}" \
      https://gitlab.rtems.org/rtems/tools/rtems-source-builder.git "${SRC}"
  fi

  echo "[setup_rtems] step 1/2: riscv toolchain (this takes a while)"
  (
    cd "${SRC}/rtems"
    ../source-builder/sb-set-builder \
      --jobs="${JOBS}" \
      --prefix="${RTEMS_HOME}/${RTEMS_VERSION}" \
      6/rtems-riscv.bset
  )

  echo "[setup_rtems] step 2/2: RTEMS 6.2 kernel + BSP ${BSP}"
  (
    cd "${SRC}/rtems"
    ../source-builder/sb-set-builder \
      --jobs="${JOBS}" \
      --prefix="${RTEMS_HOME}/${RTEMS_VERSION}" \
      6/rtems-kernel.bset \
      --with-rtems-bsp="${BSP}"
  )
fi

echo "[setup_rtems] toolchain check:"
"${RTEMS_HOME}/${RTEMS_VERSION}/bin/riscv-rtems6-gcc" --version | head -1
echo "[setup_rtems] DONE - run 'make rtems' to cross-compile frick-fsw"
