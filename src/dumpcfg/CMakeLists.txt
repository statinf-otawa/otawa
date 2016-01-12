set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib")
set(CMAKE_CXX_FLAGS "-Wall" )

set(dumpcfg_SOURCES
	"dumpcfg.cpp"
	"SimpleDisplayer.cpp"
	"DisassemblerDisplayer.cpp"
	"DotDisplayer.cpp"
	)

add_executable(dumpcfg ${dumpcfg_SOURCES})

target_link_libraries(dumpcfg otawa)
install(TARGETS dumpcfg DESTINATION bin)
