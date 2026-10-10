#!/usr/bin/env bash
# frick-fsw - emulated-target verification (Renode PolarFire SoC / QEMU).
#
# Advisory demonstration path (CI evidence: ci:emulated-run): builds the RTEMS
# MPFS image (toolchain permitting), runs it on Renode (`polarfire-soc`) and/or a
# generic `riscv/rv64imafdc` image on QEMU, and checks the boot banner. Records
# what ran; SKIPs when a tool/image is absent. Always exits 0. See
# docs/vv/emulated-target.md.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RTEMS_HOME="${RTEMS_HOME:-/opt/rtems}"
GCC="${RTEMS_HOME}/rtems/6/bin/riscv-rtems6-gcc"
EXE_MPFS="${ROOT}/build/rtems/frick_fsw"
EXE_GEN="${ROOT}/build/rtems-generic/frick_fsw"
LOG="$(mktemp)"

if [[ -x "${GCC}" ]]; then
  echo "[emulated] RTEMS MPFS cross build"
  cmake -S "${ROOT}" -B "${ROOT}/build/rtems" \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT}/cmake/toolchains/rtems-riscv-mpfs.cmake" \
    -DFF_PLATFORM=rtems > /dev/null
  cmake --build "${ROOT}/build/rtems" -j"$(nproc)" > /dev/null || echo "[emulated] MPFS cross build failed"
else
  echo "[emulated] SKIP build: RTEMS toolchain absent (run tools/scripts/setup_rtems.sh)"
fi

if command -v renode > /dev/null 2>&1 && [[ -x "${EXE_MPFS}" ]]; then
  echo "[emulated] Renode (polarfire-soc)"
  timeout 30 renode --disable-xwt -e "\
    mach create; \
    machine LoadPlatformDescription @platforms/cpus/polarfire-soc.repl; \
    showAnalyzer sysbus.uart0; \
    sysbus LoadELF @${EXE_MPFS}; \
    start; sleep 5; quit" > "${LOG}" 2>&1 || true
  grep -qi "frick-fsw" "${LOG}" && echo "[emulated] Renode boot banner: OK" \
    || echo "[emulated] Renode output captured (advisory; see ${LOG})"
else
  echo "[emulated] SKIP Renode: renode or MPFS image absent"
fi

if command -v qemu-system-riscv64 > /dev/null 2>&1 && [[ -x "${EXE_GEN}" ]]; then
  echo "[emulated] QEMU (virt, -bios; generic rv64imafdc image)"
  timeout 10 qemu-system-riscv64 -M virt -nographic -bios "${EXE_GEN}" > "${LOG}" 2>&1 || true
  grep -q "frick-fsw" "${LOG}" && echo "[emulated] QEMU boot banner: OK" \
    || echo "[emulated] QEMU banner: not found (advisory)"
else
  echo "[emulated] SKIP QEMU: qemu-system-riscv64 or generic image absent (build riscv/rv64imafdc)"
fi

echo "[emulated] done (advisory; see docs/vv/emulated-target.md)"
exit 0
