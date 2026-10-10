# frick-fsw - build system (CMake)
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only

cmake_minimum_required(VERSION 3.20)
project(frick_fsw
        VERSION 0.1.0
        DESCRIPTION "cFS-inspired flight software: PUS TM/TC on RTEMS/RISC-V (MPFS)"
        LANGUAGES C)

# ----------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------
option(FF_BUILD_TESTS   "Build the C unit test suite"             ON)
option(FF_USE_PUSOPEN  "Enable the commercial PUSopen backend"    OFF)
option(FF_ENABLE_WERROR "Treat warnings as errors"                OFF)
option(FF_ENABLE_COVERAGE "Instrument the build for gcov coverage" OFF)
option(FF_BUILD_HAL    "Build the HAL core registry (milestone M3)" ON)
option(FF_USE_LIBM     "Link the platform math library (libm, milestone M6)" ON)
option(FF_BUILD_SDLS   "Build the CCSDS SDLS security sub-layer (milestone M7)" ON)
option(FF_BUILD_DRIVERS "Build the reference device drivers (milestone M4)" ON)
option(FF_USE_LWIP    "Build the lwIP network-stack adapter (milestone M4, needs third_party/lwip)" OFF)

set(FF_PLATFORM "native" CACHE STRING "Target platform: native | rtems")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(FF_SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FF_CFG_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/config)

if(FF_PLATFORM STREQUAL "rtems")
  add_compile_definitions(FRICK_PLATFORM_MPFS)
endif()
if(FF_USE_LIBM)
  add_compile_definitions(FRICK_USE_LIBM=1)
endif()
if(FF_USE_PUSOPEN)
  add_compile_definitions(FRICK_USE_PUSOPEN=1)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pusopen)
endif()
if(FF_ENABLE_COVERAGE)
  add_compile_options(--coverage)
  add_link_options(--coverage)
endif()

# ----------------------------------------------------------------------
# Core static library (framework + reference applications)
# ----------------------------------------------------------------------
set(FF_CORE_SOURCES
  ${FF_SRC_DIR}/core/pal/pal_posix.c
  ${FF_SRC_DIR}/core/pal/pal_rtems.c
  ${FF_SRC_DIR}/core/sb/sb.c
  ${FF_SRC_DIR}/core/evs/evs.c
  ${FF_SRC_DIR}/core/es/es.c
  ${FF_SRC_DIR}/core/tbl/tbl.c
  ${FF_SRC_DIR}/core/time/time.c
  ${FF_SRC_DIR}/core/time/pps.c
  ${FF_SRC_DIR}/bsp/mpfs/board.c
  ${FF_SRC_DIR}/transport/ccsds/ccsds.c
  ${FF_SRC_DIR}/transport/link/link.c
  ${FF_SRC_DIR}/transport/link/link_loopback.c
  ${FF_SRC_DIR}/transport/link/link_uart.c
  ${FF_SRC_DIR}/pus/pus.c
  ${FF_SRC_DIR}/pus/pus_services.c
  ${FF_SRC_DIR}/pus/adapters/pus_adapter_open.c
  ${FF_SRC_DIR}/pus/adapters/pus_adapter_pusopen.c
  ${FF_SRC_DIR}/apps/app_cmd_ingest.c
  ${FF_SRC_DIR}/apps/app_tm_output.c
  ${FF_SRC_DIR}/apps/app_scheduler.c
  ${FF_SRC_DIR}/apps/app_hk.c
  ${FF_SRC_DIR}/apps/app_hs.c
  ${FF_SRC_DIR}/apps/app_sample.c
  ${CMAKE_CURRENT_SOURCE_DIR}/config/tables/fsw_tables.c
)

# HAL core registry: optional component so users can omit the abstraction layer
# they do not need (partitioning, objective OBJ-07; norm N8). Header-only
# contracts always ship; this only toggles the registry implementation.
if(FF_BUILD_HAL)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/hal/hal.c)
endif()

# Portable mathematics facade over the platform libm (milestone M6).
if(FF_USE_LIBM)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/math/ff_math.c)
endif()

# CCSDS SDLS security sub-layer (milestone M7). The cryptographic backend is
# pluggable and mission-provided; the non-secure reference backend is only used
# by the tests.
if(FF_BUILD_SDLS)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/security/ff_sdls.c)
endif()

# Ethernet driver (Cadence GEM, PolarFire SoC MSS) - milestone M4. Static,
# bounded, host-testable; the reference implementation is separable/optional.
if(FF_BUILD_DRIVERS)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/eth/gem/ff_gem.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/eth/gem/ff_gem_tsn.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/net/ff_net_loopback.c)
endif()

add_library(frick_fsw_core STATIC ${FF_CORE_SOURCES})
target_include_directories(frick_fsw_core PUBLIC ${FF_SRC_DIR} ${FF_SRC_DIR}/core ${FF_CFG_DIR})

if(NOT FF_PLATFORM STREQUAL "rtems")
  find_package(Threads REQUIRED)
  target_link_libraries(frick_fsw_core PUBLIC Threads::Threads)
endif()

if(FF_USE_LIBM)
  target_link_libraries(frick_fsw_core PUBLIC m)
endif()

# ----------------------------------------------------------------------
# lwIP network-stack adapter (milestone M4). Third-party BSD-3 stack, fetched
# pinned (tools/scripts/fetch_third_party.sh); built only with FF_USE_LWIP=ON.
# ----------------------------------------------------------------------
if(FF_USE_LWIP)
  set(FF_LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lwip)
  if(NOT EXISTS ${FF_LWIP_DIR}/src/Filelists.cmake)
    message(FATAL_ERROR "FF_USE_LWIP=ON but third_party/lwip is missing; run tools/scripts/fetch_third_party.sh")
  endif()
  set(LWIP_DIR ${FF_LWIP_DIR})
  set(LWIP_CONTRIB_DIR ${FF_LWIP_DIR}/contrib)
  set(LWIP_EXCLUDE_SLIPIF TRUE)
  include(${LWIP_CONTRIB_DIR}/ports/CMakeCommon.cmake)
  include(${LWIP_CONTRIB_DIR}/ports/unix/Filelists.cmake)
  include(${FF_LWIP_DIR}/src/Filelists.cmake)
  add_library(frick_lwip STATIC ${lwipnoapps_SRCS} ${lwipcontribportunix_SRCS})
  target_include_directories(frick_lwip PUBLIC
    ${FF_SRC_DIR}/hal/net/lwip
    ${FF_LWIP_DIR}/src/include
    ${LWIP_CONTRIB_DIR}/ports/unix/port/include)
  # Third-party lwIP is not -Werror clean: do not apply LWIP_COMPILER_FLAGS.
  find_package(Threads REQUIRED)
  target_link_libraries(frick_lwip PUBLIC Threads::Threads)

  # Our adapter, compiled with the project's strict warnings, against lwIP.
  add_library(frick_net_lwip STATIC ${FF_SRC_DIR}/hal/net/lwip/ff_net_lwip.c)
  target_include_directories(frick_net_lwip PRIVATE
    ${FF_SRC_DIR} ${FF_SRC_DIR}/core ${FF_SRC_DIR}/hal/net/lwip)
  target_compile_options(frick_net_lwip PRIVATE -Wall -Wextra)
  target_link_libraries(frick_net_lwip PUBLIC frick_lwip)
  target_link_libraries(frick_fsw_core PUBLIC frick_net_lwip)
endif()

if(FF_ENABLE_WERROR)
  target_compile_options(frick_fsw_core PRIVATE -Werror)
endif()

# ----------------------------------------------------------------------
# Executable
# ----------------------------------------------------------------------
add_executable(frick_fsw ${FF_SRC_DIR}/main/fsw_main.c)
if(NOT FF_PLATFORM STREQUAL "rtems")
  target_sources(frick_fsw PRIVATE ${FF_SRC_DIR}/main/main_native.c)
else()
  # RTEMS application configuration table (CONFIGURE_* + confdefs) + entry
  # point POSIX_Init() (in pal_rtems.c). See docs/architecture/rtems-integration.md.
  target_sources(frick_fsw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/config/rtems/rtems_config.c)
endif()
target_link_libraries(frick_fsw PRIVATE frick_fsw_core)

# ----------------------------------------------------------------------
# Tests (host only)
# ----------------------------------------------------------------------
if(FF_BUILD_TESTS AND NOT FF_PLATFORM STREQUAL "rtems")
  enable_testing()
  add_subdirectory(tests)
endif()