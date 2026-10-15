# RTEMS 6.2 cross toolchain for the Microchip RT PolarFire SoC (RTPFS160ZT)
#
# Prerequisites:
#   open-source RTEMS 6.2 (RTEMS project) installed into
#   $ENV{RTEMS_HOME}/6 (default RTEMS_HOME=/opt/rtems -> /opt/rtems/6) so that:
#     - $ENV{RTEMS_HOME}/6/bin contains riscv-rtems6-gcc
#     - BSP mpfs64imafdc is built (BSP root $ENV{RTEMS_HOME}/6/riscv-rtems6/mpfs64imafdc)
#
# See tools/scripts/setup_rtems.sh (host) and tools/docker/rtems/ (container image).
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
set(RTEMS_BSP "mpfs64imafdc")

# The BSP is a multilib variant (rv64imafdc/lp64d/medany); -mcmodel=medany is
# required to select it. The RTEMS gcc spec does not add the BSP include/lib
# paths automatically, so they are passed explicitly (verified against a real
# toolchain install; see docs/vv/verification-results.md).
set(RTEMS_BSP_ROOT ${RTEMS_HOME}/${RTEMS_VERSION}/riscv-rtems6/${RTEMS_BSP})
set(CMAKE_C_COMPILER ${RTEMS_HOME}/${RTEMS_VERSION}/bin/riscv-rtems6-gcc)
set(CMAKE_C_FLAGS_INIT "-march=rv64imafdc -mabi=lp64d -mcmodel=medany -O2 -I${RTEMS_BSP_ROOT}/lib/include")
# -qrtems selects the RTEMS libraries and linker script (linkcmds); -L points at
# the BSP libraries. Do not override the load address.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-L${RTEMS_BSP_ROOT}/lib -qrtems")

set(CMAKE_PREFIX_PATH ${RTEMS_HOME}/${RTEMS_VERSION})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH ${RTEMS_HOME}/${RTEMS_VERSION})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
