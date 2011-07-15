

set(opcg_SOURCES
	"opcg.cpp"
	)
	
add_executable(opcg ${opcg_SOURCES})

target_link_libraries(opcg ${LIBELM})
target_link_libraries(opcg libotawa)
target_link_libraries(opcg odisplay)