

set(dumpcfg_SOURCES
	"dumpcfg.cpp"
	"SimpleDisplayer.cpp"
	"DisassemblerDisplayer.cpp"
	"DotDisplayer.cpp"
	)

add_executable(dumpcfg ${dumpcfg_SOURCES})

target_link_libraries(dumpcfg libotawa)
install(TARGETS dumpcfg DESTINATION bin)
