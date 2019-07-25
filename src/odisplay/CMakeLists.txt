set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(SOURCES
	"display.cpp"
	"display_CFGDisplayer.cpp"
	"display_CFGOutput.cpp"
	"display_Displayer.cpp"
	"display_Plugin.cpp"
	"ILPSystemDisplayer.cpp"
)

include_directories(".")

add_library(display SHARED ${SOURCES})
target_link_libraries(display otawa ${LIBELM})
set_property(TARGET display PROPERTY PREFIX "")

install(TARGETS display DESTINATION ${LIBDIR}/otawa/otawa)
install(FILES display.eld DESTINATION "${LIBDIR}/otawa/otawa")
