/*
 *	$Id$
 *	DriverPlugin class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <otawa/display/Plugin.h>

namespace otawa { namespace display {

/**
 * @defgroup display Graph Display
 *
 * **module name:** `otawa/display`
 *
 * This module provides facilities to display graphs. To be displayed the
 * graph must extend @ref otawa::graph::DiGraph class and the lookup is
 * obtained from a @ref otawa::graph::Decorator class that configures the
 * output for the graph, vertices and edges.
 *
 * To maintain type safety, you can use the class
 * @ref otawa::display::GenDecoratorinstead of otawa::display::Decorator.
 *
 * The lookup of graphs, vertices and edges is made of a caption, supported
 * by class otawa::display::Text supporting usual C++ output operator
 * and special formatting tags. The style of vertices and edges is also
 * described using several style classes:
 *   * otawa::display::VertexStyle
 *   * otawa::display::EdgeStyle
 *
 * Several output format are supported depending on the available plug-ins
 * (`.dot`, postscript, PDF, image format and `view` meaning that the
 * embedded OTAWA viewer, `otaw-xdot.py`, is invoked).
 *
 * For now, the only supported plugin is `otawa/graphviz` delegating the
 * work to the [GraphViz](http://graphviz.org/) library.
 */

/**
 * @class Plugin
 * A plugin to provide driver to display graphs.
 * @ingroup display
 */


/**
 * Build the plugin.
 * @param name				Name of the plugin.
 * @param version			Version of the plugin.
 * @param plugger_version	Version of the plugger (must be OTAWA_DISPLAY_VERSION).
 */
Plugin::Plugin(
	elm::CString name,
	const elm::Version& version,
	const elm::Version& plugger_version
): elm::sys::Plugin(name, plugger_version, OTAWA_DISPLAY_NAME) {
}


/**
 * @fn Graph *Plugin::newGraph(const PropList& defaultGraphStyle, const PropList& defaultNodeStyle, const PropList& defaultEdgeStyle) const;
 * This method must return a blank empty graph ready to build a
 * displayed graph.
 * @param defaultGraphStyle	Style of the graph.
 * @param defaultNodeStyle	Default style for the nodes.
 * @param defaultEdgeStyle	Default style for the edges.
 * @return					Empty graph.
 */

} } // otawa::display
