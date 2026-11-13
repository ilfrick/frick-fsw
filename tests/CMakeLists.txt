# frick-fsw - unit test registration (CMake)
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

add_executable(test_ccsds unit/test_ccsds.c)
add_executable(test_sb    unit/test_sb.c)
add_executable(test_sb_stress unit/test_sb_stress.c)
add_executable(test_sb_sweep unit/test_sb_sweep.c)
add_executable(test_tbl   unit/test_tbl.c)
add_executable(test_sched unit/test_sched.c)
add_executable(test_sched_stats unit/test_sched_stats.c)
add_executable(test_sched_max_entries unit/test_sched_max_entries.c)
add_executable(test_sched_events unit/test_sched_events.c)
target_include_directories(test_sched_events PRIVATE ${FF_SRC_DIR}/apps ${FF_SRC_DIR}/core ${FF_CFG_DIR})
add_executable(test_pus   unit/test_pus.c)
add_executable(test_time  unit/test_time.c)
add_executable(test_frozen_utc_seam unit/test_frozen_utc_seam.c)
add_executable(test_r2_gap unit/test_r2_gap.c)
if(NOT FF_BUILD_HAL)
  target_compile_definitions(test_r2_gap PRIVATE FF_TEST_NO_HAL)
endif()
add_executable(test_r3_gap unit/test_r3_gap.c)
add_executable(test_pus_event_identity unit/test_pus_event_identity.c)
add_executable(test_tbl_sched_coupling unit/test_tbl_sched_coupling.c)
target_include_directories(test_tbl_sched_coupling PRIVATE ${FF_SRC_DIR}/apps ${FF_SRC_DIR}/core ${FF_CFG_DIR} ${FF_SRC_DIR} ${FF_SRC_DIR}/hal/net/lwip ${FF_SRC_DIR}/transport)
target_compile_definitions(test_tbl_sched_coupling PRIVATE FF_TEST_HOOKS)
target_link_libraries(test_tbl_sched_coupling PRIVATE frick_fsw_core)
add_executable(test_sched_tm_chain unit/test_sched_tm_chain.c)
target_include_directories(test_sched_tm_chain PRIVATE ${FF_SRC_DIR}/apps ${FF_SRC_DIR}/core ${FF_CFG_DIR} ${FF_SRC_DIR} ${FF_SRC_DIR}/hal/net/lwip ${FF_SRC_DIR}/transport)
target_compile_definitions(test_sched_tm_chain PRIVATE FF_TEST_HOOKS)
target_link_libraries(test_sched_tm_chain PRIVATE frick_fsw_core Threads::Threads)
#N22 - TS5 - 47 : end - to - end scheduler EVS → PUS S5 → TM sink → link loopback
#test(host SIL, FF_TEST_HOOKS time seam; Threads::Threads for pthread / nanosleep)

if(FF_PLATFORM STREQUAL "native")
#Exercise the production UART backend against a host PTY.The PTY path is a
#test - only compile - time override; no hardware / target evidence is implied.
  add_executable(test_link_uart unit/test_link_uart.c
    ${FF_SRC_DIR}/transport/link/link_uart.c)
  target_include_directories(test_link_uart PRIVATE ${FF_SRC_DIR} ${FF_SRC_DIR}/core ${FF_CFG_DIR})
  target_compile_definitions(test_link_uart PRIVATE
    FF_LINK_UART_DEVICE=\"/dev/ttyUSB0\"
    FF_LINK_UART_DEVICE_ENV=\"FRICK_TEST_UART_DEVICE\")
  target_compile_options(test_link_uart PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(test_link_uart PRIVATE Threads::Threads)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
# External syscall harness: force deterministic short/EAGAIN writes and a
# restore-fcntl failure on the UART slave descriptor, exercising the production
# partial-progress and restore-error paths without changing production code.
    target_link_options(test_link_uart PRIVATE -Wl,--wrap=write)
    target_link_options(test_link_uart PRIVATE -Wl,--wrap=fcntl)
    target_compile_definitions(test_link_uart PRIVATE
      FF_TEST_HAVE_WRITE_WRAP=1 FF_TEST_HAVE_FCNTL_WRAP=1)
  endif()
endif()

add_executable(test_es_hs_lifecycle unit/test_es_hs_lifecycle.c)
add_executable(test_evs_hs_health unit/test_evs_hs_health.c)
add_executable(test_tm_output_chain unit/test_tm_output_chain.c)
add_executable(test_pal_watchdog unit/test_pal_watchdog.c)
if(FF_PLATFORM STREQUAL "native" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_link_options(test_pal_watchdog PRIVATE -Wl,--wrap=nanosleep)
  target_compile_definitions(test_pal_watchdog PRIVATE FF_TEST_HAVE_NANOSLEEP_WRAP=1)
#Host watchdog model / contract cases(owner approval Telegram1447, 2026 - 10 - 22).
#The link - time wrappers replace the three PAL watchdog hooks for THIS target
#only, with a host test double that drives the model fixture; the
#test_pal_watchdog target keeps the production host backend and all of its
#existing oracles unchanged.HOST MODEL / PORTABLE - CONTRACT evidence only:
#not binding evidence(the MPFS / MSS binding is not wired, NC - PAL - 01) and not
#target evidence.
  add_executable(test_wd_host_contract unit/test_wd_host_contract.c
    stubs/ff_test_wd_model.c)
  target_link_options(test_wd_host_contract PRIVATE
    -Wl,--wrap=ff_pal_wd_hw_arm
    -Wl,--wrap=ff_pal_wd_hw_feed
    -Wl,--wrap=ff_pal_wd_hw_reset)

#XW - 1 abstract external - watchdog contract(docs / vv / external - watchdog - test - contract.md).
#test_extwd_model validates the test double alone; test_extwd_middleware links the
#PRODUCTION pal_watchdog.c unchanged and substitutes only the three device hooks.
#Host contract evidence only : not a device, not binding evidence(NC - PAL - 01), not
#N18, and not target evidence.
  add_executable(test_extwd_model unit/test_extwd_model.c
    stubs/ff_test_extwd_model.c)
  add_executable(test_extwd_middleware unit/test_extwd_middleware.c
    stubs/ff_test_extwd_model.c stubs/ff_test_extwd_pal.c)
  target_include_directories(test_extwd_model PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
  target_include_directories(test_extwd_middleware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
  target_link_options(test_extwd_middleware PRIVATE
    -Wl,--wrap=ff_pal_wd_hw_arm
    -Wl,--wrap=ff_pal_wd_hw_feed
    -Wl,--wrap=ff_pal_wd_hw_reset)
endif()
add_executable(test_decoder_corpus unit/test_decoder_corpus.c)
add_executable(test_pus_fu unit/test_pus_fu.c)

#S8 - PUS - FU S11 residual : S1 / S3 / S9 / S11 / S17 / S20 / S6 subservice dispatch validation
add_executable(test_pus_s11_residual unit/test_pus_s11_residual.c)

#test_r2_gap : pthread_cond_timedwait / wait error injection for PAL queue error paths
if(FF_PLATFORM STREQUAL "native" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_link_options(test_r2_gap PRIVATE -Wl,--wrap=pthread_cond_timedwait -Wl,--wrap=pthread_cond_wait)
  target_compile_definitions(test_r2_gap PRIVATE FF_TEST_HAVE_PTHREAD_COND_WRAP=1)
endif()
if(FF_USE_N7_OBCP)
  add_executable(test_obcp_integration unit/test_obcp_integration.c)
endif()

set(FF_TEST_TARGETS test_ccsds test_sb test_sb_stress test_sb_sweep test_tbl test_sched test_sched_stats test_sched_max_entries test_sched_events test_pus test_pus_fu test_pus_s11_residual test_evs_hs_health test_tm_output_chain test_pus_event_identity
                    test_time test_r2_gap test_r3_gap test_es_hs_lifecycle test_pal_watchdog
                    test_decoder_corpus test_frozen_utc_seam test_sched_tm_chain)
if(TARGET test_obcp_integration)
  list(APPEND FF_TEST_TARGETS test_obcp_integration)
  target_include_directories(test_obcp_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
if(TARGET test_wd_host_contract)
  list(APPEND FF_TEST_TARGETS test_wd_host_contract)
  target_include_directories(test_wd_host_contract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
endif()
if(TARGET test_extwd_middleware)
  list(APPEND FF_TEST_TARGETS test_extwd_model test_extwd_middleware)
endif()
if(FF_USE_PUS_ASN1)
#Exercise the configured default adapter against the actual generated N7
#headers and CRC implementation linked through frick_fsw_core.
  add_executable(test_pus_n7_adapter unit/test_pus_n7_adapter.c)
  list(APPEND FF_TEST_TARGETS test_pus_n7_adapter)
endif()
if(NOT FF_USE_PUSOPEN)
#Compile the in - repository adapter against a controlled test double.This
#checks its seam without requiring or claiming compatibility with vendor SDK.
  add_executable(test_pusopen unit/test_pusopen.c
    ${FF_SRC_DIR}/pus/adapters/pus_adapter_pusopen.c)
  target_include_directories(test_pusopen BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stubs)
  target_compile_definitions(test_pusopen PRIVATE FRICK_USE_PUSOPEN=1)
  list(APPEND FF_TEST_TARGETS test_pusopen)
endif()
if(FF_BUILD_HAL)
  add_executable(test_hal unit/test_hal.c)
  list(APPEND FF_TEST_TARGETS test_hal)
endif()
if(FF_USE_LIBM)
  add_executable(test_math unit/test_math.c)
  list(APPEND FF_TEST_TARGETS test_math)
endif()
if(FF_BUILD_SDLS)
  add_executable(test_sdls unit/test_sdls.c ${FF_SRC_DIR}/security/ff_crypto_test.c)
  list(APPEND FF_TEST_TARGETS test_sdls)
endif()
if(FF_BUILD_DRIVERS)
  add_executable(test_gem unit/test_gem.c)
  list(APPEND FF_TEST_TARGETS test_gem)
  add_executable(test_gem_tsn unit/test_gem_tsn.c)
  list(APPEND FF_TEST_TARGETS test_gem_tsn)
  add_executable(test_gptp unit/test_gptp.c)
  list(APPEND FF_TEST_TARGETS test_gptp)
  add_executable(test_netloop unit/test_netloop.c)
  list(APPEND FF_TEST_TARGETS test_netloop)
  add_executable(test_gpio unit/test_gpio.c)
  list(APPEND FF_TEST_TARGETS test_gpio)
  add_executable(test_serial unit/test_serial.c)
  list(APPEND FF_TEST_TARGETS test_serial)
  add_executable(test_mem unit/test_mem.c)
  list(APPEND FF_TEST_TARGETS test_mem)
  add_executable(test_nvm unit/test_nvm.c)
  list(APPEND FF_TEST_TARGETS test_nvm)
  add_executable(test_mtd unit/test_mtd.c)
  list(APPEND FF_TEST_TARGETS test_mtd)
  add_executable(test_can unit/test_can.c)
  list(APPEND FF_TEST_TARGETS test_can)
  add_executable(test_i2c unit/test_i2c.c)
  list(APPEND FF_TEST_TARGETS test_i2c)
  add_executable(test_spi unit/test_spi.c)
  list(APPEND FF_TEST_TARGETS test_spi)
  add_executable(test_block unit/test_block.c)
  list(APPEND FF_TEST_TARGETS test_block)
  add_executable(test_pcie unit/test_pcie.c)
  list(APPEND FF_TEST_TARGETS test_pcie)
  add_executable(test_spacewire unit/test_spacewire.c)
  list(APPEND FF_TEST_TARGETS test_spacewire)
  add_executable(test_fabric unit/test_fabric.c)
  list(APPEND FF_TEST_TARGETS test_fabric)
  add_executable(test_canfd unit/test_canfd.c)
  list(APPEND FF_TEST_TARGETS test_canfd)
  add_executable(test_canopen unit/test_canopen.c)
  list(APPEND FF_TEST_TARGETS test_canopen)
  add_executable(test_link_ip unit/test_link_ip.c)
  list(APPEND FF_TEST_TARGETS test_link_ip)
  add_executable(test_link_tsn unit/test_link_tsn.c)
  list(APPEND FF_TEST_TARGETS test_link_tsn)
endif()
if(FF_USE_LWIP)
  add_executable(test_lwip unit/test_lwip.c)
  add_executable(test_lwip_netif unit/test_lwip_netif.c)
  list(APPEND FF_TEST_TARGETS test_lwip test_lwip_netif)
endif()

foreach(t ${FF_TEST_TARGETS})
  target_link_libraries(${t} PRIVATE frick_fsw_core)
endforeach()

#Strict warning gate for the test scripts(norm N9 / M9 quality gate).The tests
#are our own code : compile them with - Wall - Wextra - Werror so the
#"-Wall -Wextra -Werror clean" evidence claim(docs / vv / verification - results.md)
#is produced by the default build and by CI, not asserted out - of - band.
foreach(t ${FF_TEST_TARGETS})
  target_compile_options(${t} PRIVATE -Wall -Wextra -Werror)
endforeach()

add_test(NAME ccsds COMMAND test_ccsds)
add_test(NAME sb    COMMAND test_sb)
add_test(NAME sb_stress COMMAND test_sb_stress)
add_test(NAME sb_sweep_boundary COMMAND test_sb_sweep 32 32)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(
  NAME sb_evs_structural_sweep
  COMMAND ${Python3_EXECUTABLE}
          ${CMAKE_SOURCE_DIR}/tools/scripts/sb_evs_structural_sweep.py
          $<TARGET_FILE:test_sb_sweep>
          --output ${CMAKE_BINARY_DIR}/sb-evs-structural-sweep.csv
)
set_tests_properties(sb_evs_structural_sweep PROPERTIES TIMEOUT 180 LABELS "tool-qualification;sb-evs")
add_test(NAME tbl   COMMAND test_tbl)
add_test(NAME sched COMMAND test_sched)
add_test(NAME sched_stats COMMAND test_sched_stats)
add_test(NAME sched_max_entries COMMAND test_sched_max_entries)
add_test(NAME sched_events COMMAND test_sched_events)
add_test(NAME pus   COMMAND test_pus)
add_test(NAME pus_fu COMMAND test_pus_fu)
add_test(NAME pus_s11_residual COMMAND test_pus_s11_residual)
if(TARGET test_obcp_integration)
  add_test(NAME obcp_engine_connectivity COMMAND test_obcp_integration)
  set_tests_properties(obcp_engine_connectivity PROPERTIES TIMEOUT 30)
endif()
if(TARGET test_pus_n7_adapter)
  add_test(NAME pus_n7_adapter_contract COMMAND test_pus_n7_adapter)
endif()
if(TARGET test_pusopen)
  add_test(NAME pusopen_adapter_contract COMMAND test_pusopen)
endif()
add_test(NAME time  COMMAND test_time)
add_test(NAME frozen_utc_seam COMMAND test_frozen_utc_seam)
add_test(NAME r2_gap COMMAND test_r2_gap)
add_test(NAME r3_gap COMMAND test_r3_gap)
add_test(NAME tbl_sched_coupling COMMAND test_tbl_sched_coupling)
add_test(NAME sched_tm_chain COMMAND test_sched_tm_chain)
set_tests_properties(sched_tm_chain PROPERTIES LABELS "n22;s8-sch")
add_test(NAME es_hs_lifecycle COMMAND test_es_hs_lifecycle)
add_test(NAME evs_hs_health COMMAND test_evs_hs_health)
set_tests_properties(evs_hs_health PROPERTIES LABELS "n22")
add_test(NAME tm_output_chain COMMAND test_tm_output_chain)
set_tests_properties(tm_output_chain PROPERTIES LABELS "n22")
add_test(NAME pus_event_identity COMMAND test_pus_event_identity)
set_tests_properties(pus_event_identity PROPERTIES TIMEOUT 60 LABELS "n22;pus;nc-ext-237")
add_test(NAME pal_watchdog COMMAND test_pal_watchdog)
if(TARGET test_wd_host_contract)
#Host watchdog model / contract cases.The label set deliberately omits
#"n22" : the case is an integration test under N22's definition (the
#production PAL plus a backend implementation across the ff_pal_wd_hw_ *
#interface, with nominal and out - of - bound legs and explicit expected
#results), but it is NOT requirement - based and covers no controlled
#requirement, hence no traceability link is created for it.No gate
#consumes the "n22" label, so no gate is weakened by its omission.
  add_test(NAME wd_host_model_contract COMMAND test_wd_host_contract)
  set_tests_properties(wd_host_model_contract PROPERTIES TIMEOUT 60
    LABELS "fdir;watchdog;host-model")
endif()
if(TARGET test_extwd_middleware)
#XW - 1 abstract external - watchdog contract.Unlike the internal - model case
#above, these cases ARE requirement - based : the oracles are the SRS - REQ - 073
#and SRS - REQ - 075 clauses(arm / service / expiry / reset obligations), mapped in
#docs / vv / test - spec / ts - 7 - xw2 - target - contract.md and traceability.yaml.
  add_test(NAME extwd_model_contract COMMAND test_extwd_model)
  add_test(NAME extwd_middleware_contract COMMAND test_extwd_middleware)
  set_tests_properties(extwd_model_contract extwd_middleware_contract
    PROPERTIES TIMEOUT 60 LABELS "n22;fdir;watchdog;xw1")
endif()
add_test(NAME decoder_corpus COMMAND test_decoder_corpus)
#Enforced wall - clock budget(N22) : a hang is a failure, not a silent pass.
#Observed runtimes are ~0.1 s / ~0.05 s, so 60 s is a large but bounded margin.
set_tests_properties(es_hs_lifecycle PROPERTIES TIMEOUT 60 LABELS "n22;s8-vv-fu;es-hs-lifecycle")
set_tests_properties(pal_watchdog PROPERTIES TIMEOUT 60 LABELS "n22;fdir;watchdog")
set_tests_properties(decoder_corpus PROPERTIES TIMEOUT 60 LABELS "n22;s8-vv-fu;decoder-corpus")
if(TARGET test_link_uart)
  add_test(NAME link_uart COMMAND test_link_uart)
  set_tests_properties(link_uart PROPERTIES TIMEOUT 5 LABELS "link;uart;host-pty")
endif()
#Regression test for the N18 Renode runner verdict logic(NC - EXT - 199) : the
#runner must never report PASS from a stale or unwritable log path.
add_test(NAME renode_vv_verdict
  COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/shell/test_renode_vv_verdict.sh)
set_tests_properties(renode_vv_verdict PROPERTIES TIMEOUT 60 LABELS "n22;vv;renode;tooling")
set_tests_properties(sched_max_entries PROPERTIES TIMEOUT 30 LABELS "n22;s8-sch;max-entries")
if(FF_BUILD_HAL)
  add_test(NAME hal COMMAND test_hal)
endif()
if(FF_USE_LIBM)
  add_test(NAME math COMMAND test_math)
endif()
if(FF_BUILD_SDLS)
  add_test(NAME sdls COMMAND test_sdls)
endif()
if(FF_BUILD_DRIVERS)
  add_test(NAME gem COMMAND test_gem)
  add_test(NAME gem_tsn COMMAND test_gem_tsn)
  add_test(NAME gptp COMMAND test_gptp)
  add_test(NAME netloop COMMAND test_netloop)
  add_test(NAME gpio COMMAND test_gpio)
  add_test(NAME serial COMMAND test_serial)
  add_test(NAME mem COMMAND test_mem)
  add_test(NAME nvm COMMAND test_nvm)
  add_test(NAME mtd COMMAND test_mtd)
  add_test(NAME can COMMAND test_can)
  add_test(NAME i2c COMMAND test_i2c)
  add_test(NAME spi COMMAND test_spi)
  add_test(NAME block COMMAND test_block)
  add_test(NAME pcie COMMAND test_pcie)
  add_test(NAME spacewire COMMAND test_spacewire)
  add_test(NAME fabric COMMAND test_fabric)
  add_test(NAME canfd COMMAND test_canfd)
  add_test(NAME canopen COMMAND test_canopen)
  add_test(NAME link_ip COMMAND test_link_ip)
  add_test(NAME link_tsn COMMAND test_link_tsn)
endif()
if(FF_USE_LWIP)
  add_test(NAME lwip COMMAND test_lwip)
  add_test(NAME lwip_netif COMMAND test_lwip_netif)
endif()
