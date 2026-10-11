#!/usr/bin/env bash
# frick-fsw - QEMU emulated-target verification (generic riscv/rv64imafdc BSP).
#
# Milestone M10 / SRS-REQ-043 (ci:emulated-run): runs the committed generic-BSP
# RTEMS image on the QEMU `virt` machine (headless), captures the UART console
# and checks the boot banner and the reference-application registration count. It
# complements the Renode PolarFire SoC functional validation (make renode, norm
# N18): the generic build carries no MPFS peripherals, so it establishes the RTEMS
# boot/PAL/tasking and the framework bring-up only.
#
# The image is built with `make qemu-image`/`qemu-image-docker` (toolchain image
# has qemu + the rv64imafdc BSP). `timeout` bounds the QEMU process so the script
# can never hang. See docs/vv/emulated-target.md.
#
# Exit semantics (norm N10: a check that can fail):
#   * image absent, or no QEMU runtime available  -> SKIP (exit 0)
#   * QEMU ran but the banner / app count missing  -> FAIL (exit 1)
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="frick-rtems-toolchain:local"
ELF="${ROOT}/build/rtems-generic/frick_fsw"
LOG="/tmp/frick-qemu.log"
CONSOLE="${ROOT}/build/rtems-generic/qemu-console.log"
APPS_EXPECTED="6 applications registered"

if [[ ! -f "${ELF}" ]]; then
  echo "[qemu] SKIP: generic-BSP ELF absent (${ELF}); build with 'make qemu-image'"
  exit 0
fi

mkdir -p "$(dirname "${CONSOLE}")"
: > "${LOG}"

if docker image inspect "${IMG}" > /dev/null 2>&1; then
  # Record the exact toolchain image used (evidence provenance).
  echo "[qemu] toolchain image ${IMG} id $(docker image inspect -f '{{.Id}}' "${IMG}")" | tee -a "${LOG}"
  docker run --rm -v "${ROOT}:/workspace" -w /workspace "${IMG}" \
    bash -lc 'timeout 20 qemu-system-riscv64 -M virt -nographic -bios build/rtems-generic/frick_fsw' \
    >> "${LOG}" 2>&1 || true
elif command -v qemu-system-riscv64 > /dev/null 2>&1; then
  timeout 20 qemu-system-riscv64 -M virt -nographic -bios "${ELF}" >> "${LOG}" 2>&1 || true
else
  echo "[qemu] SKIP: no QEMU runtime (toolchain image or qemu-system-riscv64 absent)"
  exit 0
fi

cp "${LOG}" "${CONSOLE}" 2>/dev/null || true

# The console carries ANSI/control characters; match with `grep -a`. A QEMU that
# started but did not reach the banner/app count is a FAIL (the ELF is present).
if grep -aq "frick-fsw" "${LOG}" && grep -aq "${APPS_EXPECTED}" "${LOG}"; then
  echo "[qemu] PASS (banner + ${APPS_EXPECTED})"
  exit 0
fi
echo "[qemu] FAIL: QEMU ran but the banner / '${APPS_EXPECTED}' marker is missing (see ${LOG})"
exit 1
