/*
 *	$Id$
 *	otawa::display concepts
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
 * A decorator is a concept that provides static functions used to display
 * a graph.
 * @param G		Type of the graph.
 * */
template <class G>
class Decorator {
public:
	
	/**
	 * This method is called to decorate the graph.
	 * @param graph		Current graph.
	 * @param caption	Output text here to set the graph caption.
	 * @param fill		Fill style of the back of the graph.
	 * @param text		Text style of the graph caption.
	 */
	static void decorate(
		const G& graph,
		Output& caption,
		TextStyle& text,
		FillStyle& fill);
	
	/**
	 * This method is called for each vertex of the graph to get its
	 * decoration.
	 * @param vertex	Current vertex.
	 * @param content	Text content of the node (output it here). To have
	 * 					a title and a body, output the title, "---\n", then
	 * 					the body.
	 * @param shape		Shape style of the vertex.
	 */
	static void decorate(
		const G& graph,
		const typename G::Vertex vertex,
		Output& content,
		ShapeStyle& style);
	
	/**
	 * This method is called for each edge.
	 * @param label		Label of the edge (output text here).
	 * @param text		Text style of the edge label.
	 * @param line		Line style of the edge.
	 */
	static void decorate(
		const G& graph,
		const typename G::Edge edge,
		Output& label,
		TextStyle& text,
		LineStyle& line);
};

} } // otawa::display
