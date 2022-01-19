set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/proc/otawa;${ORIGIN}/../lib/otawa/otawa")
add_executable(test_dom "test_dom.cpp")
target_link_libraries(test_dom otawa ${LIBELM})

add_test(test_dom_bs test_dom ../benchs/bs.elf)
add_test(test_dom_crc test_dom ../benchs/crc.elf)
add_test(test_dom_multi test_dom ../benchs/multi.elf)
