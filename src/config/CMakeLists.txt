set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

add_executable(otawa-config "config.cpp")

target_link_libraries(otawa-config otawa)
install(TARGETS otawa-config DESTINATION bin)

if(APPLE)
	set_target_properties(otawa-config PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
