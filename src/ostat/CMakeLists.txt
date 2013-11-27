set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib/")

set(ostat_SOURCES
	"ostat.cpp"
	)
	
add_executable(ostat ${ostat_SOURCES})

target_link_libraries(ostat ${LIBELM})
target_link_libraries(ostat otawa)

install(TARGETS ostat DESTINATION bin)

if(APPLE)
	set_target_properties(ostat PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
