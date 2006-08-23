/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/display_Graph.cpp -- display::Graph class implementation.
 */

#include <otawa/display/Graph.h>

namespace otawa { namespace display {

/**
 * Tells that the properties with the given identifier have to be printed
 */
GenericIdentifier<Identifier*> INCLUDE("display.include");
/**
 * Tells that the properties with the given identifier mustn't be printed
 */
GenericIdentifier<Identifier*> EXCLUDE("display.exclude");
/**
 * The value of the property must be either INCLUDE or EXCLUDE.
 * Gives a default behaviour to the properties that doesn't appear in
 * any of the INCLUDE / EXCLUDE lists.
 */
GenericIdentifier<Identifier*> DEFAULT("display.default");
/**
 * Identifier of the background color
 */
GenericIdentifier<elm::CString> BACKGROUND("display.background");
/**
 * Identifier of the drawing color (boxes, edges)
 */
GenericIdentifier<elm::CString> COLOR("display.color");
/**
 * Identifier of the drawing style.
 * Must be one of @ref style_t
 */
GenericIdentifier<int> STYLE("display.style");
/**
 * Identifier of the text color. It is the same Identifier as TEXT_COLOR
 */
GenericIdentifier<elm::CString> FONT_COLOR("display.text_color");
/**
 * Identifier of the text color. It is the same Identifier as FONT_COLOR
 */
GenericIdentifier<elm::CString> &TEXT_COLOR = FONT_COLOR;
/**
 * Identifier of the text size. It is the same Identifier as TEXT_SIZE
 */
GenericIdentifier<int>  FONT_SIZE("display.text_size");
/**
 * Identifier of the text size. It is the same Identifier as FONT_SIZE
 */
GenericIdentifier<int>& TEXT_SIZE = FONT_SIZE;
/**
 * Identifier of the font name
 */
GenericIdentifier<elm::CString> FONT("display.font_name");
/**
 * Identifier of the url of the link the object is pointing to
 */
GenericIdentifier<elm::CString> HREF("display.href");
/**
 * Identifier of the title of a node
 */
GenericIdentifier<elm::String> TITLE("display.title");
/**
 * Identifier of the body of a node
 */
GenericIdentifier<elm::String> BODY("display.body");
/**
 * Identifier of a shape of a node. Must be one of @ref shape_t
 */
GenericIdentifier<int> SHAPE("display.shape");
/**
 * Identifier of a label of an edge
 */
GenericIdentifier<elm::String> LABEL("display.label");
/**
 * Identifier of a weight of an edge
 */
GenericIdentifier<int> WEIGHT("display.weight");



/**
 * @fn Item::setProps(const PropList& props)
 * Set the object properties that have to be printed
 */



/**
 * @fn Graph::newNode(const PropList& style = PropList::EMPTY, const PropList& props = PropList::EMPTY)
 * This function creates a new node in the graph
 */



/**
 * @fn Graph::newEdge(Node *source, Node *target, const PropList& style = PropList::EMPTY, const PropList& props = PropList::EMPTY)
 * This function creates a new edge between the two given nodes, in the graph
 */



/**
 * @fn Graph::display(void)
 * This functions displays the graph
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
 * @page odisplay ODisplay Library
 * 
 * This library is delivered with OTAWA to provide graphical display
 * of managed graphs.
 * 
 * The main principle is to provides the same interface
 * to each graph application whatever the required final output using a
 * display driver system.
 * 
 * Using abstract graph description using classes
 * @ref otawa::display::Graph, @ref otawa::display::Node or
 * @ref otawa::display::Edge, the applications describes the graph and the
 * driver is responsible for outputting the graph in the user-chosen
 * form (vector graphics, bitmap, screen display and so on).
 */


 
} }

