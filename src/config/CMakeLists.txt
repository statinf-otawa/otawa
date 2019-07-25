set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib")

set(CMAKE_CXX_FLAGS "-Wall" )
add_executable(otawa-config "config.cpp")

target_link_libraries(otawa-config otawa)
install(TARGETS otawa-config DESTINATION bin)
