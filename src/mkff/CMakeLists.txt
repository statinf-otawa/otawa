set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib:${ORIGIN}/../lib/otawa/proc/otawa")
set(CMAKE_CXX_FLAGS "-Wall" )


add_executable(mkff mkff.cpp)
target_link_libraries(mkff ${LIBELM} dynbranch odisplay)
target_link_libraries(mkff otawa)
install(TARGETS mkff DESTINATION bin)
