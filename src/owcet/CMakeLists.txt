set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib/")

set(owcet_SOURCES
	"owcet.cpp"
	)
	
add_executable(owcet ${owcet_SOURCES})

target_link_libraries(owcet ${LIBELM})
target_link_libraries(owcet otawa)

install(TARGETS owcet DESTINATION bin)

if(APPLE)
	set_target_properties(owcet PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
