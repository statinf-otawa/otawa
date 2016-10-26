set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/proc/otawa;${ORIGIN}/../lib/otawa/otawa")
set(CMAKE_CXX_FLAGS "-Wall" )

set(odfa_SOURCES "odfa.cpp")
add_executable(odfa ${odfa_SOURCES})
target_link_libraries(odfa otawa display clp)
install(TARGETS odfa DESTINATION bin)
