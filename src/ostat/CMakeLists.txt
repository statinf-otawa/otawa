set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib/")
set(CMAKE_CXX_FLAGS "-Wall" )

set(ostat_SOURCES
	"ostat.cpp"
	)

add_executable(ostat ${ostat_SOURCES})

target_link_libraries(ostat ${LIBELM})
target_link_libraries(ostat otawa)

install(TARGETS ostat DESTINATION bin)
