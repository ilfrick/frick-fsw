add_executable(test_ccsds unit/test_ccsds.c)
add_executable(test_sb    unit/test_sb.c)
add_executable(test_tbl   unit/test_tbl.c)
add_executable(test_pus   unit/test_pus.c)

foreach(t test_ccsds test_sb test_tbl test_pus)
  target_link_libraries(${t} PRIVATE frick_fsw_core)
endforeach()

add_test(NAME ccsds COMMAND test_ccsds)
add_test(NAME sb    COMMAND test_sb)
add_test(NAME tbl   COMMAND test_tbl)
add_test(NAME pus   COMMAND test_pus)