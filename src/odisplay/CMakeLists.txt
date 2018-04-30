set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

set(SOURCES
	"CFGDrawer.cpp"
	"display.cpp"
	"display_AbstractDrawer.cpp"
	"display_CFGAdapter.cpp"
	"display_CFGDisplayer.cpp"
#	"display_InlinedCFGDisplayer.cpp"
	"display_CFGOutput.cpp"
	"display_Displayer.cpp"
	"display_GenDrawer.cpp"
	"display_Graph.cpp"
	"display_graphviz.cpp"
	"display_Plugin.cpp"
	"graphviz.h"
	"graphviz_Edge.cpp"
	"graphviz_Graph.cpp"
	"graphviz_GraphElement.cpp"
	"graphviz_Item.cpp"
	"graphviz_Node.cpp"
	"graphviz_quote.cpp"
	"ILPSystemDisplayer.cpp"
	"utility.cpp"
)

include_directories(".")

add_library(display SHARED ${SOURCES})
target_link_libraries(display otawa ${LIBELM})
set_property(TARGET display PROPERTY PREFIX "")

install(TARGETS display DESTINATION ${LIBDIR}/otawa/otawa)
install(FILES display.eld DESTINATION "${LIBDIR}/otawa/otawa")
