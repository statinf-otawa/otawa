/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/graphviz_Edge.cpp -- GraphVizEdge class implementation.
 */
#include "graphviz.h"


namespace otawa { namespace display {



/**
 * @author G. Cavaignac
 * @class GraphVizEdge
 * This class represents the links between the graphviz nodes in the graphviz graph
 */


/**
 * Creates a new edge with the first node given as source node,
 * and the second node as target node.
 * @param src source node
 * @param dest target node
 */
GraphVizEdge::GraphVizEdge(GraphVizNode *src, GraphVizNode *dest)
: _hasLabel(false), _src(src), _dest(dest){
}



/**
 * @fn GraphVizEdge::source()
 * This function returns the source of this edge
 * @return source of this edge
 */



/**
 * @fn GraphVizEdge::target()
 * This function returns the target of this edge
 * @return target of this edge
 */




String GraphVizEdge::attributes(){
	return GraphVizItem::attributes(*this);
}



void GraphVizEdge::printOthersAttributes(elm::io::Output& out){
	String props = getPropertiesString();
	if(props.length() > 0 || _hasLabel){
		out << "label=\"";
		if(_hasLabel){
			out << quoteSpecials(_label) << "\\l";
		}
		if(props.length() > 0){
			out << quoteSpecials(props) << "\\l";
		}
		out << '"';
	}
}



bool GraphVizEdge::printAttribute(elm::io::Output &out, const PropList::Iter& prop){
	if(prop == LABEL){
		_hasLabel = true;
		_label = LABEL.get(prop);
		return false;
	}
	else if(prop == WEIGHT){
		out << "weight=" << WEIGHT.get(prop);
		return true;
	}
	return GraphVizGraphElement::printAttribute(out, prop);
}



void GraphVizEdge::setProps(const PropList& props){
	GraphVizItem::setProps(props);
}



} }


