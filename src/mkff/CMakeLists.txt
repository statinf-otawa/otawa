set(CMAKE_INSTALL_RPATH "../lib/\\$ORIGIN")


set(mkff_SOURCES
	mkff.cpp
	)
	
# making exe here ?
add_executable(mkff ${mkff_SOURCES})

target_link_libraries(mkff ${LIBELM})
target_link_libraries(mkff otawa)

install(TARGETS mkff DESTINATION bin)
