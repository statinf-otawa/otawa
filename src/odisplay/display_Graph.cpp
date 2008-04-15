/*
 *	$Id$
 *	Driver class implementation
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

#include <otawa/display/Graph.h>
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>

namespace otawa { namespace display {

/**
 * Tells that the properties with the given identifier have to be printed
 * @ingroup display
 */
Identifier<AbstractIdentifier*> INCLUDE("otawa::display::include");


/**
 * Tells that the properties with the given identifier mustn't be printed
 * @ingroup display
 */
Identifier<AbstractIdentifier*> EXCLUDE("otawa::display::exclude");


/**
 * The value of the property must be either INCLUDE or EXCLUDE.
 * Gives a default behaviour to the properties that doesn't appear in
 * any of the INCLUDE / EXCLUDE lists.
 * @ingroup display
 */
Identifier<AbstractIdentifier*> DEFAULT("otawa::display::default");


/**
 * Identifier of the background color
 * @ingroup display
 */
Identifier<elm::CString> BACKGROUND("otawa::display::background");


/**
 * Identifier of the drawing color (boxes, edges)
 * @ingroup display
 */
Identifier<elm::CString> COLOR("otawa::display::color");


/**
 * Identifier of the drawing style.
 * Must be one of @ref style_t
 * @ingroup display
 */
Identifier<int> STYLE("otawa::display::style");


/**
 * Identifier of the text color. It is the same Identifier as TEXT_COLOR
 * @ingroup display
 */
Identifier<elm::CString> FONT_COLOR("otawa::display::text_color");


/**
 * Identifier of the text color. It is the same Identifier as FONT_COLOR
 * @ingroup display
 */
Identifier<elm::CString> &TEXT_COLOR = FONT_COLOR;


/**
 * Identifier of the text size. It is the same Identifier as TEXT_SIZE
 * @ingroup display
 */
Identifier<int>  FONT_SIZE("otawa::display::text_size");


/**
 * Identifier of the text size. It is the same Identifier as FONT_SIZE
 * @ingroup display
 */
Identifier<int>& TEXT_SIZE = FONT_SIZE;


/**
 * Identifier of the font name
 * @ingroup display
 */
Identifier<elm::CString> FONT("otawa::display::font_name");


/**
 * Identifier of the url of the link the object is pointing to
 * @ingroup display
 */
Identifier<elm::CString> HREF("otawa::display::href");


/**
 * Identifier of the title of a node
 * @ingroup display
 */
Identifier<elm::String> TITLE("otawa::display::title");


/**
 * Identifier of the body of a node
 * @ingroup display
 */
Identifier<elm::String> BODY("otawa::display::body");


/**
 * Identifier of a shape of a node. Must be one of @ref shape_t
 * @ingroup display
 */
Identifier<int> SHAPE("otawa::display::shape");


/**
 * Identifier of a label of an edge
 * @ingroup display
 */
Identifier<elm::String> LABEL("otawa::display::label");


/**
 * Identifier of a weight of an edge
 * @ingroup display
 */
Identifier<int> WEIGHT("otawa::display::weight");


/**
 * @class Item
 * This class is the base of @ref Graph, @ref Node and @ref Edge. It contains
 * facilities to display the graph.
 * @see @ref odisplay
 * @ingroup display
 */


/**
 * @fn Item::setProps(const PropList& props)
 * Set the object properties that have to be printed.
 * @param props	Set the properties to print.
 */


/**
 * @class Graph
 * A graph provides facilities to display a graph. newNode() and newEdge()
 * methods  builds the graph that is displayed when the display() method is
 * called.
 * @see @ref odisplay
 * @ingroup display
 */


/**
 * @fn Graph::newNode(const PropList& style, const PropList& props)
 * This function creates a new node in the graph
 * @param style	Style of the node.
 * @param props	Properties to display.
 */



/**
 * @fn Graph::newEdge(Node *source, Node *target, const PropList& style, const PropList& props)
 * This function creates a new edge between the two given nodes, in the graph
 * @param source	Source node of the edge.
 * @param target	Target node of the edge.
 * @param style		Style of the node.
 * @param props		Properties to display.
 */



/**
 * @fn Graph::display(void)
 * This functions displays the graph
 */


/**
 * @class Driver
 * A driver provides facilities to display a graph. The newGraph() method
 * build a graph that may be populated according the needs of the graph to
 * display.
 * @see @ref odisplay
 * @ingroup display
 */


/**
 * @fn Driver::newGraph(const PropList& defaultGraphStyle, const PropList& defaultNodeStyle, const PropList& defaultEdgeStyle) const
 * Creates a new graph with default styles given
 * @param defaultGraphStyle default properties for the graph
 * Can be changed with the methods inherited from PropList
 * @param defaultNodeStyle default properties for the nodes
 * Can be changed with the methods inherited from PropList
 * @param defaultEdgeStyle default properties for the edges
 * Can be changed with the methods inherited from PropList
 * @return new graph created
 */


/**
 * Find a driver by its name.
 * @param name	Name of the driver to look for.
 * @return		Driver matching the name or the default driver if an empty
 * 				string is passed.
 */
Driver *Driver::find(string name) {
	if(!name || name == "graphviz")
		return &graphviz_driver;
	else
		return 0;
}


/**
 * Find a driver by its output kind.
 * @param kind	Kind of the output to produce.
 * @return		Found driver.
 */
Driver *find(kind_t kind) {
	return &graphviz_driver;
}


/**
 * Passed to the graph style of the Driver::newGraph() method, selects the
 * kind of output.
 * @ingroup display
 */
Identifier<kind_t> OUTPUT_KIND("otawa::display::output_kind", OUTPUT_ANY);


/**
 * Passed to the graph style of the Driver::newGraph() method, selects the
 * file to output to when the driver display in a file.
 * @ingroup display
 */
Identifier<string> OUTPUT_PATH("otawa::display::output_path", "");


/**
 * @defgroup display ODisplay Library
 * 
 * This library is delivered with OTAWA to provide graphical display
 * of managed graphs.
 * 
 * The main principle is to provides the same interface
 * to each graph application whatever the required final output using a
 * display driver system.
 * 
 * @par Classes
 * Using abstract graph description using classes
 * @ref otawa::display::Graph, @ref otawa::display::Node or
 * @ref otawa::display::Edge, the applications describes the graph and the
 * driver is responsible for outputting the graph in the user-chosen
 * form (vector graphics, bitmap, screen display and so on).
 * 
 * @par Linking with ODisplay
 * The linkage options for using the ODisplay library may be easily obtained
 * by using otawa-config:
 * @code
 * $ otawa-config --libs display
 * @endcode
 * 
 * @par Graph Look
 * 
 * The look of the graph is described using a set of object allowing
 * to set the different display attributes.
 * 
 * @li @ref display::LineStyle : define the style and the color of the
 * lines (supported by edges and nodes),
 * @li @ref display::TextStyle : define the style, the color and the font
 * of the displayed text (supported by edge labels, node contents and graph
 * caption),
 * @li @ref display::FillStyle : define the fill style and the color
 * of the background (supported by graph and nodes),
 * @li @ref display::ShapeStyle : define the shape of a node
 * (includes the TextStyle, the FillStyle and the LineStyle).
 * 
 * @par GenDrawer class
 * 
 * The Gendrawer<G, D> class is the main entry point to use the display module.
 * It accepts two generic arguments. The first one is the type of the
 * displayed graph and must implement the concept
 * @ref otawa::concept::DiGraphWithNodeMap. The second one gives the look
 * of each item of the graph and must implement the concept display::Decorator.
 * 
 * To display a graph, you have to :
 * @li provide a Decorator class,
 * @li build the GenDrawer object,
 * @li configure this object,
 * @li launch the drawing using the GenDrawer::draw() method.
 * 
 * In the example below, we draw a CFG from OTAWA. As the CFG does not implement
 * the DiGraphWithNodeMap concept, we use te display::CFGAdapter to wrap it.
 * The nodes of the graph is displayed with their basic block number except
 * for entry and exit that are described with diamond and name. The edges
 * are displayed using dashed lines.
 * 
 * @code
 * #include <otawa/cfg.h>
 * #include <otawa/display/GenDrawer.h>
 * #include <otawa/display/CFGAdapter.h>
 * using namespace otawa::display;
 * 
 * class MyDecorator {
 * public:
 * class CFGDecorator {
 * public:
 *	static void decorate(
 *		const G& graph,
 *		const display::CFGAdapter& graph,
 *		Output& caption,
 *		display::TextStyle& text,
 *		display::FillStyle& fill)
 *	{ caption << graph->label() << " CFG"; }
 *	
 *	static void decorate(
*                 const G& graph,
 *		const display::CFGAdapter::Vertex vertex,
 *		Output& content,
 *		display::ShapeStyle& style)
 *	{
 * 		if(vertex->isEntry() || vertex->isExit())
 * 			style.shape = ShapeStyle::SHAPE_DIAMOND;
 * 		if(vertex->isEntry()) content << "ENTRY";
 * 		else if(vertex->isExit()) content << "EXIT";
 * 		else content << vertex->number();
 *	}
 *
 *	static void decorate(
 *		const display::CFGAdapter::Edge edge,
 *		Output& label,
 *		display::TextStyle& text,
 *		display::LineStyle& line)
 *	{ line.style = LineStyle::DASHED;
 * };
 * 
 * CFG *my_cfg;
 * CFGAdapter adapter(my_cfg);
 * GenDrawer<CFGAdapter, MyDecorator> drawer(adapter);
 * drawer.path = "my_cfg.ps";
 * drawer.draw();
 * @endcode
 * 
 * There exist other adapters like the display::GenGraphAdapter for the
 * GenGraph class.
 * 
 * @par Old Properties
 * This system of properties to describe the graph look is still supported but
 * deprecated. It used in the CFGDrawer class.
 * 
 * Most classes of this library use a common set of properties described here.
 * 
 * The first set concerns the selection of the displayed attributes.
 * @li @ref INCLUDE - takes an identifier to include in the display,
 * @li @ref EXCLUDE - takes an identifier to exclude in the display,
 * @li @ref DEFAULT - set the default behaviour (@ref INCLUDE or @ref EXCLUDE).
 * 
 * Other properties allow to select the lookup of the displayed elements:
 * @li @ref BACKGROUND - background color,
 * @li @ref COLOR - draw color,
 * @li @ref STYLE - text style,
 * @li @ref FONT_COLOR, @ref TEXT_COLOR - text color,
 * @li @ref FONT_SIZE, TEXT_SIZE - text size,
 * @li @ref FONT - text font
 * 
 * We have properties that describes the content of the object:
 * @li @ref HREF - hypertext reference,
 * @li @ref TITLE - title of the element,
 * @li @ref BODY - body of the element,
 * @li @ref SHAPE - shape of the element,
 * @li @ref LABEL - label for an edge,
 * @li @ref WEIGHT - weight for an edge.
 * 
 * Finally, some properties select the kind of output:
 * @li @ref OUTPUT_KIND
 * @li @ref OUTPUT_PATH
 * 
 * @par GraphViz Driver
 * ODisplay is delivered with a driver for the GraphViz package using the DOT
 * textual format. This is the default driver and it accepts some specialized
 * properties:
 * @li @ref GRAPHVIZ_LAYOUT - layout of the displayed graph.
 */


/**
 * @class DisplayException
 * This exception is thrown during the display of a graph if there is an error.
 * @ingroup display
 */

} }

