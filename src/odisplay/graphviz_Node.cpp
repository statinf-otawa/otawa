/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/graphviz_Node.cpp -- GraphVizNode class implementation.
 */
#include "graphviz.h"


namespace otawa { namespace display {


/**
 * @author G. Cavaignac
 * @class GraphVizNode
 * This class represents the nodes in the graphviz graph
 */



/**
 * Creates a new node with the given number.
 * One must give a different number each time, or the nodes
 * could be confused in the graph
 * @param number node number
 */
GraphVizNode::GraphVizNode(int number)
: _hasTitle(false), _hasBody(false), _shapeAcceptsBody(false), _number(number) {
}



/**
 * @fn GraphVizNode::number()
 * This function returns the number of this node
 * @return number of this node
 */



String GraphVizNode::attributes(){
	return GraphVizItem::attributes(*this);
}



void GraphVizNode::setProps(const PropList& props){
	GraphVizItem::setProps(props);
}



void GraphVizNode::printOthersAttributes(elm::io::Output& out){
	String props = getPropertiesString();
	if(_hasTitle || _hasBody || props.length() > 0){
		out << "label=\"";
		if(_shapeAcceptsBody){
			out << '{' << quoteSpecials(_title);
			if(_hasBody){
				out << '|' << quoteSpecials(_body) << "\\l";
			}
			if(props.length() > 0){
				out << '|' << quoteSpecials(props) << "\\l";
			}
			out << '}';
		}
		else {
			out << quoteSpecials(_title);
			if(_hasTitle && (_hasBody || props.length() > 0))
				out << '\n';
			if(_hasBody){
				out << quoteSpecials(_body) << "\\l";
			}
			if(props.length() > 0){
				out << quoteSpecials(props) << "\\l";
			}
		}
		out << '"';
	}
}



bool GraphVizNode::printAttribute(elm::io::Output &out, const PropList::Iter& prop){
	if(prop == TITLE){
		_title = prop.get<String>();
		//cerr << "TITLE = " << _title << '\n';
		_hasTitle = true;
		return false;
	}
	else if(prop == BODY){
		_body = prop.get<String>();
		_hasBody = true;
		return false;
	}
	else if(prop == SHAPE){
		_shapeAcceptsBody = false;
		out << "shape=";
		switch(prop.get<int>()){
			case ShapeStyle::SHAPE_RECORD:
				out << "record";
				_shapeAcceptsBody = true;
				break;
			case ShapeStyle::SHAPE_MRECORD:
				out << "Mrecord";
				_shapeAcceptsBody = true;
				break;
			case ShapeStyle::SHAPE_BOX:
				out << "box";
				break;
			case ShapeStyle::SHAPE_CIRCLE:
				out << "circle";
				break;
			case ShapeStyle::SHAPE_ELLIPSE:
				out << "ellipse";
				break;
			case ShapeStyle::SHAPE_EGG:
				out << "egg";
				break;
			case ShapeStyle::SHAPE_TRIANGLE:
				out << "triangle";
				break;
			case ShapeStyle::SHAPE_TRAPEZIUM:
				out << "trapezium";
				break;
			case ShapeStyle::SHAPE_PARALLELOGRAM:
				out << "parallelogram";
				break;
			case ShapeStyle::SHAPE_HEXAGON:
				out << "hexagon";
				break;
			case ShapeStyle::SHAPE_OCTAGON:
				out << "octagon";
				break;
			case ShapeStyle::SHAPE_DIAMOND:
				out << "diamond";
				break;
			default:
				assert(false);
		}
		return true;
	}
	return GraphVizGraphElement::printAttribute(out, prop);
}



} }

