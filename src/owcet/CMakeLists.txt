set(CMAKE_INSTALL_RPATH "../lib/\\$ORIGIN")

set(owcet_SOURCES
	"owcet.cpp"
	)
	
add_executable(owcet ${owcet_SOURCES})

target_link_libraries(owcet ${LIBELM})
target_link_libraries(owcet otawa)

install(TARGETS owcet DESTINATION bin)
