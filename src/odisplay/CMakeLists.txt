set(CMAKE_INSTALL_RPATH "\\$ORIGIN")
set(CMAKE_MACOSX_RPATH true)

set(libodisplay_la_SOURCES
	"CFGDrawer.cpp"
	"display.cpp"
	"display_AbstractDrawer.cpp"
	"display_CFGAdapter.cpp"
	"display_CFGOutput.cpp"
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

add_library(odisplay SHARED ${libodisplay_la_SOURCES})
install(TARGETS odisplay DESTINATION ${LIBDIR})

target_link_libraries(odisplay otawa)
target_link_libraries(odisplay ${LIBELM})

install(FILES display.eld DESTINATION "${LIBDIR}/otawa/proc/otawa")
