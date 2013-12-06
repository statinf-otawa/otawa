set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../lib")

set(dumpcfg_SOURCES
	"dumpcfg.cpp"
	"SimpleDisplayer.cpp"
	"DisassemblerDisplayer.cpp"
	"DotDisplayer.cpp"
	)

add_executable(dumpcfg ${dumpcfg_SOURCES})

target_link_libraries(dumpcfg otawa)
install(TARGETS dumpcfg DESTINATION bin)

if(APPLE)
	set_target_properties(dumpcfg PROPERTIES INSTALL_RPATH "@executable_path/../lib")
endif()
