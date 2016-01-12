set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib")
set(CMAKE_CXX_FLAGS "-Wall" )

set(odfa_SOURCES "odfa.cpp")
add_executable(odfa ${odfa_SOURCES})
target_link_libraries(odfa otawa odisplay)
install(TARGETS odfa DESTINATION bin)
