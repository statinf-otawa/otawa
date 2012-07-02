set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

add_executable(otawa-config "config.cpp")

target_link_libraries(otawa-config otawa)
install(TARGETS otawa-config DESTINATION bin)
