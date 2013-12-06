set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

set(odfa_SOURCES "odfa.cpp")
add_executable(odfa ${odfa_SOURCES})
target_link_libraries(odfa otawa odisplay)
install(TARGETS odfa DESTINATION bin)

if(APPLE)
	set_target_properties(odfa PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
