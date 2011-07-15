

set(oipet_SOURCES
	"oipet.cpp"
	)
	
add_executable(oipet ${oipet_SOURCES})

target_link_libraries(oipet ${LIBELM})
target_link_libraries(oipet libotawa)
target_link_libraries(oipet odisplay)
