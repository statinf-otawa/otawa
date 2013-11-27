set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

# Need to check what a "noinst_PROGRAMS" is.

set(odisasm_SOURCES
	"odisasm.cpp"
)
	
add_executable(odisasm ${odisasm_SOURCES})
target_link_libraries(odisasm ${LIBELM})
target_link_libraries(odisasm otawa)
install(TARGETS odisasm DESTINATION bin)

if(APPLE)
	set_target_properties(odisasm PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
