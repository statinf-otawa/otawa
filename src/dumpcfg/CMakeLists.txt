set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib;${ORIGIN}/../lib/otawa/otawa")
set(CMAKE_CXX_FLAGS "-Wall" )

set(dumpcfg_SOURCES
	"dumpcfg.cpp"
	"SimpleDisplayer.cpp"
	"DisassemblerDisplayer.cpp"
	"DotDisplayer.cpp"
	"MultipleDotDisplayer.cpp"
)

add_executable(dumpcfg ${dumpcfg_SOURCES})
target_link_libraries(dumpcfg otawa)
target_link_libraries(dumpcfg display)
install(TARGETS dumpcfg DESTINATION bin)
