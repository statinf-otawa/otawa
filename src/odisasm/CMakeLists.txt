set(CMAKE_INSTALL_RPATH "../lib/\\$ORIGIN")

# Need to check what a "noinst_PROGRAMS" is.

set(odisasm_SOURCES
	"odisasm.cpp"
	)
	
add_executable(odisasm ${odisasm_SOURCES})

target_link_libraries(odisasm ${LIBELM})
target_link_libraries(odisasm otawa)
