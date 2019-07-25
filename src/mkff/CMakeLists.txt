set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/proc/otawa;${ORIGIN}/../lib/otawa/otawa")
set(CMAKE_CXX_FLAGS "-Wall" )


add_executable(mkff mkff.cpp display_MultipleDotDisplayer.cpp display_MKFFDotDisplayer.cpp)
target_link_libraries(mkff otawa ${LIBELM} display)
install(TARGETS mkff DESTINATION bin)
