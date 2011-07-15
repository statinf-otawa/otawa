
set(ostat_SOURCES
	"ostat.cpp"
	)
	
add_executable(ostat ${ostat_SOURCES})

target_link_libraries(ostat ${LIBELM})
target_link_libraries(ostat libotawa)