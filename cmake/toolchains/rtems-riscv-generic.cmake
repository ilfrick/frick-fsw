# RTEMS 6.2 cross toolchain for the generic RISC-V `riscv/rv64imafdc` BSP.
#
# Used only for the QEMU (virt) emulated-target verification path (milestone
# M10, SRS-REQ-043): the generic BSP boots under `qemu-system-riscv64 -M virt`,
# complementing the MPFS image run on Renode. It is NOT the flight target (the
# flight BSP is riscv/mpfs64imafdc, see rtems-riscv-mpfs.cmake).
#
# Prerequisites: the RTEMS 6.2 toolchain with BSP rv64imafdc built into
# $ENV{RTEMS_HOME}/6 (tools/docker/rtems/, target `make rtems-toolchain`).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)
set(CMAKE_CROSSCOMPILING TRUE)

set(RTEMS_HOME $ENV{RTEMS_HOME})
if(NOT RTEMS_HOME)
  set(RTEMS_HOME "/opt/rtems")
endif()

set(RTEMS_VERSION "6")
set(RTEMS_BSP "rv64imafdc")

set(RTEMS_BSP_ROOT ${RTEMS_HOME}/${RTEMS_VERSION}/riscv-rtems6/${RTEMS_BSP})
set(CMAKE_C_COMPILER ${RTEMS_HOME}/${RTEMS_VERSION}/bin/riscv-rtems6-gcc)
set(CMAKE_C_FLAGS_INIT "-march=rv64imafdc -mabi=lp64d -mcmodel=medany -O2 -I${RTEMS_BSP_ROOT}/lib/include")
# -qrtems selects the RTEMS libraries and linker script (linkcmds); -L points at
# the BSP libraries. Do not override the load address (the generic BSP linkcmds
# places the image where QEMU `virt` maps RAM).
set(CMAKE_EXE_LINKER_FLAGS_INIT "-L${RTEMS_BSP_ROOT}/lib -qrtems")

set(CMAKE_PREFIX_PATH ${RTEMS_HOME}/${RTEMS_VERSION})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH ${RTEMS_HOME}/${RTEMS_VERSION})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
