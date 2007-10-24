/*
 *	$Id$
 *	graphviz class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "graphviz.h"
#include <otawa/properties.h>

using namespace elm::genstruct;

namespace otawa { namespace display {

/**
 * Identifier for the dot layout. Must be one of @ref layout_t.
 * Default is LAYOUT_DOT (standard directional graph, from up to down)
 */
Identifier<int> GRAPHVIZ_LAYOUT("otawa::display::graphviz_layout", LAYOUT_DOT);


/**
 * @class GraphVizDriver
 * @author G. Cavaignac
 * This class is the driver for making graphviz graphs
 */


/**
 * Build the driver.
 */
GraphVizDriver::GraphVizDriver(void)
: Plugin("graphviz", Version(1, 0, 0), OTAWA_DISPLAY_VERSION) {	
}


/**
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
