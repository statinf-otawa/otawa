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
extern Identifier<AbstractIdentifier*> INCLUDE;
extern Identifier<AbstractIdentifier*> EXCLUDE;

// either INCLUDE or EXCLUDE
extern Identifier<AbstractIdentifier*> DEFAULT;

// Item class
class Item: public PropList {
public:
	virtual void setProps(const PropList& props) = 0;
};
extern Identifier<elm::CString> BACKGROUND;
extern Identifier<elm::CString> COLOR;

// draw_style_t enum
typedef enum style_t {
	STYLE_NONE = 0,
	STYLE_FILLED,
	STYLE_PLAIN,
	STYLE_DASHED,
	STYLE_DOTTED,
	STYLE_BOLD
} style_t;
extern Identifier<int> STYLE;
extern Identifier<elm::CString>  FONT_COLOR;
extern Identifier<elm::CString>& TEXT_COLOR;
extern Identifier<int>  FONT_SIZE;
extern Identifier<int>& TEXT_SIZE;
extern Identifier<elm::CString> FONT;
extern Identifier<elm::CString> HREF;



// Node class
class Node: public Item {
};
extern Identifier<elm::String> TITLE;
extern Identifier<elm::String> BODY;

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
extern Identifier<int> SHAPE;


// Edge class
class Edge: public Item {
public:
};
extern Identifier<int> WEIGHT;
extern Identifier<elm::String> LABEL;

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
