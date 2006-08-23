/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/display/Graph.h -- display::Graph class interface.
 */
#ifndef OTAWA_DISPLAY_GRAPH_H
#define OTAWA_DISPLAY_GRAPH_H

#include <otawa/properties.h>

namespace otawa { namespace display {

// Filters
extern GenericIdentifier<Identifier*> INCLUDE;
extern GenericIdentifier<Identifier*> EXCLUDE;

// either INCLUDE or EXCLUDE
extern GenericIdentifier<Identifier*> DEFAULT;

// Item class
class Item: public PropList {
public:
	virtual void setProps(const PropList& props) = 0;
};
extern GenericIdentifier<elm::CString> BACKGROUND;
extern GenericIdentifier<elm::CString> COLOR;

// draw_style_t enum
typedef enum style_t {
	STYLE_NONE = 0,
	STYLE_FILLED,
	STYLE_PLAIN,
	STYLE_DASHED,
	STYLE_DOTTED,
	STYLE_BOLD
} style_t;
extern GenericIdentifier<int> STYLE;
extern GenericIdentifier<elm::CString>  FONT_COLOR;
extern GenericIdentifier<elm::CString>& TEXT_COLOR;
extern GenericIdentifier<int>  FONT_SIZE;
extern GenericIdentifier<int>& TEXT_SIZE;
extern GenericIdentifier<elm::CString> FONT;
extern GenericIdentifier<elm::CString> HREF;



// Node class
class Node: public Item {
};
extern GenericIdentifier<elm::String> TITLE;
extern GenericIdentifier<elm::String> BODY;

// form_t enum
typedef enum shape_t {
	SHAPE_NONE = 0,
	SHAPE_RECORD,
	SHAPE_MRECORD,
	SHAPE_BOX,
	SHAPE_CIRCLE,
	SHAPE_ELLIPSE,
	SHAPE_EGG,
	SHAPE_TRIANGLE,
	SHAPE_TRAPEZIUM,
	SHAPE_PARALLELOGRAM,
	SHAPE_HEXAGON,
	SHAPE_OCTAGON,
	SHAPE_DIAMOND
} shape_t;
extern GenericIdentifier<int> SHAPE;


// Edge class
class Edge: public Item {
public:
};
extern GenericIdentifier<int> WEIGHT;
extern GenericIdentifier<elm::String> LABEL;

// Graph class
class Graph: public Item {
public:
	virtual Node *newNode(const PropList& style = PropList::EMPTY,
		const PropList& props = PropList::EMPTY) = 0;
	virtual Edge *newEdge(Node *source, Node *target,
		const PropList& style = PropList::EMPTY,
		const PropList& props = PropList::EMPTY) = 0;
	virtual void display(void) = 0;
};

} } // otawa::display

#endif	//	OTAWA_DISPLAY_GRAPH_H
