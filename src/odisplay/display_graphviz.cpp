/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/display_graphviz.cpp -- graphviz module principal source file.
 */
#include "graphviz.h"
#include <otawa/properties.h>

using namespace elm::genstruct;

namespace otawa { namespace display {

/**
 * Identifier for the dot layout. Must be one of @ref layout_t.
 * Default is LAYOUT_DOT (standard directional graph, from up to down)
 */
Identifier<int> GRAPHVIZ_LAYOUT("display.graphviz.layout", LAYOUT_DOT);
/**
 * Identifier for the output. Must be one of @ref output_t.
 * Default is OUTOUT_PS (PostScript format)
 */
Identifier<int> GRAPHVIZ_OUTPUT("display.graphviz.output", OUTPUT_PS);
/**
 * Identifier for the file that will be created.
 * Default is "odisplay.ps"
 */
Identifier<elm::CString> GRAPHVIZ_FILE("display.graphviz.file", "odisplay.ps");





/**
 * @author G. Cavaignac
 * @class GraphVizDriver
 * This class is the driver for making graphviz graphs
 */



Graph *GraphVizDriver::newGraph(
	const PropList& defaultGraphStyle,
	const PropList& defaultNodeStyle,
	const PropList& defaultEdgeStyle) const
{
	return new GraphVizGraph(defaultGraphStyle, defaultNodeStyle, defaultEdgeStyle);
}




/**
 * The GraphVizDriver itself
 */
static GraphVizDriver driver;

/**
 * The graphviz driver
 */
Driver& graphviz_driver = driver;

} }

