set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

set(odfa_SOURCES
	"odfa.cpp"
)
	
add_executable(odfa ${odfa_SOURCES})
target_link_libraries(odfa ${LIBELM})
target_link_libraries(odfa otawa)
install(TARGETS odfa DESTINATION bin)
