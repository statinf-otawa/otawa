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

// draw_style_t enum
typedef enum draw_style_t {
	DRAW_NONE = 0,
	DRAW_PLAIN,
	DRAW_DASHED,
	DRAW_DOTTED,
	DRAW_BOLD
} draw_style_t;


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
	SHAPE_TRAPZIUM,
	SHAPE_PARALLELOGRAM,
	SHAPE_HEXAGON,
	SHAPE_OCTAGON,
	SHAPE_DIAMOND
} shape_t;


// Item class
class Item {
public:
	virtual void putStyle(const PropList& props) = 0;
	virtual void setProps(const PropList& props) = 0;
};
extern GenericIdentifier<elm::CString> BACKGROUND;
extern GenericIdentifier<elm::CString> DRAW_COLOR;
extern GenericIdentifier<int> DRAW_STYLE;
extern GenericIdentifier<int> TEXT_COLOR;
extern GenericIdentifier<int> TEXT_SIZE;
extern GenericIdentifier<elm::String&> HREF;


// Node class
class Node: public Item {
};
extern GenericIdentifier<elm::String&> TITLE;
extern GenericIdentifier<elm::String&> BODY;
extern GenericIdentifier<int> SHAPE;


// Edge class
class Edge: public Item {
public:
};
extern GenericIdentifier<elm::String&> LABEL;
extern GenericIdentifier<int> WEIGHT;


// Graph class
class Graph: public Item {
public:
	virtual Node *newNode(const PropList& props = PropList::EMPTY,
		const PropList& style = PropList::EMPTY) = 0;
	virtual Edge *newEdge(Node *source, Node *target,
		const PropList& props = PropList::EMPTY,
		const PropList& style = PropList::EMPTY) = 0;
	virtual void display(void) = 0;
};

} } // otawa::display

#endif	//	OTAWA_DISPLAY_GRAPH_H
