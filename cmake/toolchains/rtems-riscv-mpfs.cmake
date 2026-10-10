# RTEMS 6.2 cross toolchain for the Microchip RT PolarFire SoC (RTPFS160ZT)
#
# Prerequisites:
#   RTEMS 6.2 source builder (rtems-source-builder) installed into
#   $ENV{RTEMS_HOME}/rtems/6 so that:
#     - $ENV{RTEMS_HOME}/rtems/6/bin contains riscv-rtems6-gcc
#     - BSP riscv/mpfs64imafdc is built
#
# See tools/scripts/setup_rtems.sh
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)
set(CMAKE_CROSSCOMPILING TRUE)

set(RTEMS_HOME $ENV{RTEMS_HOME})
if(NOT RTEMS_HOME)
  set(RTEMS_HOME "/opt/rtems")
endif()

set(RTEMS_VERSION "6")
set(RTEMS_BSP "riscv/mpfs64imafdc")

set(CMAKE_SYSROOT ${RTEMS_HOME}/rtems/${RTEMS_VERSION}/${RTEMS_BSP})
set(CMAKE_PREFIX_PATH ${CMAKE_SYSROOT})

set(CMAKE_C_COMPILER ${RTEMS_HOME}/rtems/${RTEMS_VERSION}/bin/riscv-rtems6-gcc)
set(CMAKE_C_FLAGS_INIT "-mcpu=e54 -mabi=lp64d -mcmodel=medany -O2")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-Ttext,0x80200000")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)