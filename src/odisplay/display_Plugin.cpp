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
): elm::system::Plugin(name, plugger_version, OTAWA_DISPLAY_NAME) {
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
