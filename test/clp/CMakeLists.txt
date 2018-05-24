set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/proc/otawa;${ORIGIN}/../lib/otawa/otawa")
add_executable(test_clp "test_clp.cpp")
target_link_libraries(test_clp otawa ${LIBELM} clp)

add_test(test_clp test_clp)
