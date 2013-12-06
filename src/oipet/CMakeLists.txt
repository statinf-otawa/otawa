set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")


set(oipet_SOURCES
	"oipet.cpp"
	)
	
add_executable(oipet ${oipet_SOURCES})

target_link_libraries(oipet ${LIBELM})
target_link_libraries(oipet otawa)
target_link_libraries(oipet odisplay)

install(TARGETS oipet DESTINATION bin)

if(APPLE)
	set_target_properties(oipet PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
