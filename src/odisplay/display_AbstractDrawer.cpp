/*
 *	$Id$
 *	AbstractDrawer class implementation
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

#include <otawa/display/AbstractDrawer.h>

namespace otawa { namespace display {

// Set the fill style
static void setFillStyle(PropList *props, const FillStyle& style) {
	if(style.color.asText())
		BACKGROUND(props) = style.color.asText().toCString();
}

// Set the text style
static void setTextStyle(PropList *props, const TextStyle& style) {
	if(style.name)
		FONT(props) = style.name.toCString();
	if(style.color.asText())
		FONT_COLOR(props) = style.color.asText().toCString();
	if(style.size)
		FONT_SIZE(props) = style.size;
}

// Set the line style
static void setLineStyle(PropList *props, const LineStyle& style) {
	if(style.color.asText())
		COLOR(props) = style.color.asText().toCString();
	if(style.weight)
		WEIGHT(props) = style.weight;
	switch(style.style) {
	case LineStyle::HIDDEN:
		STYLE(props) = STYLE_NONE;
		break;
	case LineStyle::PLAIN:
		break;
	case LineStyle::DOTTED:
		STYLE(props) = STYLE_DOTTED;
		break;
	case LineStyle::DASHED:
		STYLE(props) = STYLE_DASHED;
		break;
	}
}

// Set the shap style
static void setShapeStyle(PropList *props, const ShapeStyle& style) {
	if(style.shape)
		SHAPE(props) = style.shape;
	setFillStyle(props, style.fill);
	setLineStyle(props, style.line);
	setTextStyle(props, style.text);
}


/**
 * @class AbstractDrawer
 * Provides facilities to draw a graph. The vertices and edges of the graph
 * must be built from the classes @ref AbstractDrawer::Vertex and
 * @ref AbstractDrawer::Edge.
 * 
 * AbstractDrawer is rarely used as is. You may prefer automatic graph drawing
 * facilities based on AbstractDrawer provides by @ref GenDrawer.
 * 
 * The drawing is configured using several publicly accessible member attributes:
 * @li kind					Kind of drawing
 * @li path					path of the file to create
 * @li page_fill			page backrgound fill style
 * @li page_text			page text style
 * @li default_vertex		default vertex shape style
 * @li default_edge_line	default edge line style
 * @li default_edge_text	default edge label text style
 * @ingroup display
 */


/**
 * Build a drawer with the default driver.
 * @throws display::Exception	If there is no driver available.
 */
AbstractDrawer::AbstractDrawer(void):
	kind(OUTPUT_ANY)
{
	Driver *driver = Driver::find();
	if(!driver)
		throw new Exception("no display driver available");
	graph = driver->newGraph();
	ASSERT(graph);
}


/**
 * Build a drawer with the given driver.
 * @param driver	Driver to use.
 */
AbstractDrawer::AbstractDrawer(Driver& driver): kind(OUTPUT_DOT) {
	graph = driver.newGraph();
	ASSERT(graph);
}


/**
 * Launch the draw of the graph.
 */
void AbstractDrawer::draw(void) {
	
	// Setup the graph
	StringBuffer buf;
	configure(buf, page_text, page_fill);
	string legend = buf.toString();
	if(legend)
		BODY(graph) = legend;
	setFillStyle(graph, page_fill);
	setTextStyle(graph, page_text);
	if(path)
		OUTPUT_PATH(graph) = path;
	if(kind)
		OUTPUT_KIND(graph) = kind;
	
	// Setup the vertices and edges
	for(FragTable<Vertex *>::Iterator vertex(vertices); vertex; vertex++)
		vertex->setup();
	for(FragTable<Edge *>::Iterator edge(edges); edge; edge++)
		edge->setup();
	
	// Draw
	graph->display();
}


/**
 * This method may be overriden to provide a customized graph ledend.
 * @param out	Graph caption.
 * @param text	Graph caption text style to configure.
 * @param fill	Graph background fill style to configure.
 */
void AbstractDrawer::configure(
	Output& caption,
	TextStyle& text,
	FillStyle& fill
) {
}


/**
 * @class AbstractDrawer::Vertex
 * This class represents vertices in the @ref AbstractDrawer.
 * @ingroup display
 */


/**
 * Build a vertex.
 * @param drawer	Parent abstract drawer.
 */
AbstractDrawer::Vertex::Vertex(AbstractDrawer& drawer) {
	node = drawer.graph->newNode();
	drawer.vertices.add(this);
	ASSERT(node);
	shape = drawer.default_vertex;
}


/**
 * This method may be overriden to provide a customized displayed text.
 * @param content	Vertex content.
 * @param shape		Vertex shape style for configuration.
 */
void AbstractDrawer::Vertex::configure(Output& content, ShapeStyle& shape) {
}


/**
 */
void AbstractDrawer::Vertex::setup(void) {
	StringBuffer buf;
	configure(buf, shape);
	setShapeStyle(node, shape);
	string title = buf.toString();
	if(title) {
		int pos = title.indexOf("\n---\n");
		if(pos >= 0) {
			BODY(node) = title.substring(pos + 5);
			title = title.substring(0, pos + 1);
		}
		TITLE(node) = title;
	}
}


/**
 * @var AbstractDrawer::Vertex::shape;
 * Shape of the vertex publicly accessible for customization.
 * @ingroup display
 */


/**
 * @class AbstractDrawer::Edge
 * An edge in a @ref AbstractDrawer.
 * @ingroup display
 */


/**
 * Build an edge.
 * @param drawer	Owner drawer.
 * @param source	Source vertex.
 * @param sink		Sink vertex.
 */
AbstractDrawer::Edge::Edge(
	AbstractDrawer& drawer,
	Vertex *source,
	Vertex *sink
) {
	edge = drawer.graph->newEdge(source->node, sink->node);
	drawer.edges.add(this);
	text = drawer.default_edge_text;
	line = drawer.default_edge_line;
}



/**
 * This method may be overriden to provide a customized displayed text.
 * @param label	Label of the edge.
 * @param text	Label text style to configure.
 * @param line	Edge line style to configure.
 */
void AbstractDrawer::Edge::configure(
	Output& label,
	TextStyle& text,
	LineStyle& line
) {
}


/**
 */
void AbstractDrawer::Edge::setup(void) {
	StringBuffer buf;
	configure(buf, text, line);
	setLineStyle(edge, line);
	string label = buf.toString();
	if(label) {
		LABEL(edge) = label;
		setTextStyle(edge, text);
	}
}


/**
 * @var AbstractDrawer::Edge::text;
 * Style of the text publicly accessible for customization.
 */


/**
 * @var AbstractDrawer::Edge::line;
 * Style of the line publicly accessible for customization.
 */

} }		// otawa::display
