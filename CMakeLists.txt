cmake_minimum_required(VERSION 3.20)
project(frick_fsw
        VERSION 0.1.0
        DESCRIPTION "cFS-inspired flight software: PUS TM/TC on RTEMS/RISC-V (MPFS)"
        LANGUAGES C)

# ----------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------
option(FF_BUILD_TESTS  "Build the C unit test suite"             ON)
option(FF_USE_PUSOPEN  "Enable the commercial PUSopen backend"    OFF)
option(FF_ENABLE_WERROR "Treat warnings as errors"                OFF)

set(FF_PLATFORM "native" CACHE STRING "Target platform: native | rtems")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(FF_SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FF_CFG_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/config)

if(FF_PLATFORM STREQUAL "rtems")
  add_compile_definitions(FRICK_PLATFORM_MPFS)
endif()
if(FF_USE_PUSOPEN)
  add_compile_definitions(FRICK_USE_PUSOPEN=1)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pusopen)
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

add_library(frick_fsw_core STATIC ${FF_CORE_SOURCES})
target_include_directories(frick_fsw_core PUBLIC ${FF_SRC_DIR} ${FF_SRC_DIR}/core ${FF_CFG_DIR})

if(NOT FF_PLATFORM STREQUAL "rtems")
  find_package(Threads REQUIRED)
  target_link_libraries(frick_fsw_core PUBLIC Threads::Threads)
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
endif()
target_link_libraries(frick_fsw PRIVATE frick_fsw_core)

# ----------------------------------------------------------------------
# Tests (host only)
# ----------------------------------------------------------------------
if(FF_BUILD_TESTS AND NOT FF_PLATFORM STREQUAL "rtems")
  enable_testing()
  add_subdirectory(tests)
endif()