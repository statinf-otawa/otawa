set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib")
set(CMAKE_CXX_FLAGS "-Wall" )


set(opcg_SOURCES
	"opcg.cpp"
	)

add_executable(opcg ${opcg_SOURCES})

target_link_libraries(opcg ${LIBELM})
target_link_libraries(opcg otawa)
target_link_libraries(opcg odisplay)

install(TARGETS opcg DESTINATION bin)
