/*
 *	$Id$
 *	GenDrawer class implementation
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

#include <otawa/display/GenDrawer.h>

namespace otawa { namespace display {

/**
 * @class GenDrawer
 * A simple engine to draw graphs.
 * @param G		Type of the graph to use (must implement the
 * 				@ref otawa::concept::DiGraphWithNodeMap concept).
 * @param D		Type describing the decorator of the nodes and edges of the
 * 				graph (must implement the @ref otawa::display::Decorator
 * 				concept).
 * @ingroup display
 */


/**
 * @fn GenDrawer::GenDrawer(const G& graph);
 * Build the drawer.
 * @param graph		Graph to work on.
 */


/**
 * @class DefaultDecorator
 * Default decorator that just output the given edges and vertices.
 * This class implements the @ref otawa::display::Decorator concept.
 * @param G		Type of the graph.
 * @ingroup display
 */

} } // otawa::display
