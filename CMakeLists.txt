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
option(FF_ENABLE_RENODE_TEST "Build the Renode functional test application (norm N18)" OFF)
option(FF_USE_PUS_ASN1 "Use the N7/ESA asn1-pusc-lib PUS-C ASN.1 default backend (needs generated code; see make pus-asn1)" ON)

set(FF_PLATFORM "native" CACHE STRING "Target platform: native | rtems | rtems-generic")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(FF_SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FF_CFG_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/config)

if(FF_PLATFORM STREQUAL "rtems")
  add_compile_definitions(FRICK_PLATFORM_MPFS)
elseif(FF_PLATFORM STREQUAL "rtems-generic")
  # Generic RTEMS BSP (riscv/rv64imafdc) for the QEMU emulated-target path (M10):
  # no PolarFire SoC board module / MPFS MMIO; see config/platform/platform_qemu.h.
  add_compile_definitions(FRICK_PLATFORM_QEMU)
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
if(FF_USE_PUS_ASN1)
  set(FF_PUS_ASN1_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asn1-pusc-lib)
  set(FF_PUS_ASN1_GEN ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asn1-pusc-lib-generated/common)
  if((NOT EXISTS "${FF_PUS_ASN1_GEN}/BasicTypes.h") OR (NOT EXISTS "${FF_PUS_ASN1_LIB}/wrappers/c/Crc.c"))
    message(WARNING
      "FF_USE_PUS_ASN1=ON (default) but the generated PUS-C ASN.1 code / N7 wrappers are missing; "
      "falling back to the open CCSDS backend. Run 'make pus-asn1' to enable the PUS-C ASN.1 default.")
    set(FF_USE_PUS_ASN1 FALSE)
  else()
    add_compile_definitions(FF_USE_PUS_ASN1=1)
    message(STATUS "PUS-C ASN.1 (N7/ESA) default backend ENABLED")
  endif()
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

# Renode functional test application (norm N18): compiled and registered only
# when enabled, so the default/flight reference set stays at six applications.
if(FF_ENABLE_RENODE_TEST)
  add_compile_definitions(FF_ENABLE_RENODE_TEST=1)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/apps/app_renode_test.c)
endif()

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
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/gpio/mpfs/ff_gpio_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/serial/mpfs/ff_serial_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/mem/ff_mem_static.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/nvm/mpfs/ff_nvm_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/mtd/mpfs/ff_mtd_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/can/mpfs/ff_can_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/i2c/mpfs/ff_i2c_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/spi/mpfs/ff_spi_mpfs.c)
  list(APPEND FF_CORE_SOURCES ${FF_SRC_DIR}/drivers/block/mpfs/ff_block_mpfs.c)
endif()

add_library(frick_fsw_core STATIC ${FF_CORE_SOURCES})
target_include_directories(frick_fsw_core PUBLIC ${FF_SRC_DIR} ${FF_SRC_DIR}/core ${FF_CFG_DIR})

if(NOT FF_PLATFORM MATCHES "^rtems")
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

# PUS-C ASN.1 (N7 Space / ESA) default backend (SRS-REQ-050). Build-time code
# generation: the C is produced by `make pus-asn1` (tools/scripts/gen_pus_asn1.sh)
# from the pinned asn1-pusc-lib model with the ESA ASN1SCC compiler; nothing
# generated or third-party is committed. Disable with -DFF_USE_PUS_ASN1=OFF to
# fall back to the open CCSDS backend.
if(FF_USE_PUS_ASN1)
  add_library(frick_pus_asn1 STATIC
    ${FF_PUS_ASN1_GEN}/BasicTypes.c
    ${FF_PUS_ASN1_GEN}/asn1crt.c
    ${FF_PUS_ASN1_GEN}/asn1crt_encoding.c
    ${FF_PUS_ASN1_GEN}/asn1crt_encoding_acn.c
    ${FF_PUS_ASN1_LIB}/wrappers/c/Crc.c
    ${FF_PUS_ASN1_LIB}/wrappers/c/IsoChecksum.c)
  target_include_directories(frick_pus_asn1 PUBLIC
    ${FF_PUS_ASN1_GEN} ${FF_PUS_ASN1_LIB}/wrappers/c)
  # Generated + N7/ESA wrapper code is third-party: do not impose our warnings.
  target_compile_options(frick_pus_asn1 PRIVATE -w)
  target_link_libraries(frick_fsw_core PUBLIC frick_pus_asn1)
  # The N7 backend adapter is part of the core (it uses the generated headers).
  target_include_directories(frick_fsw_core PRIVATE
    ${FF_PUS_ASN1_GEN} ${FF_PUS_ASN1_LIB}/wrappers/c)
  target_sources(frick_fsw_core PRIVATE ${FF_SRC_DIR}/pus/adapters/pus_adapter_n7.c)
endif()

if(FF_ENABLE_WERROR)
  target_compile_options(frick_fsw_core PRIVATE -Werror)
endif()

# ----------------------------------------------------------------------
# Executable
# ----------------------------------------------------------------------
add_executable(frick_fsw ${FF_SRC_DIR}/main/fsw_main.c)
if(NOT FF_PLATFORM MATCHES "^rtems")
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
if(FF_BUILD_TESTS AND NOT FF_PLATFORM MATCHES "^rtems")
  enable_testing()
  add_subdirectory(tests)
endif()
