/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/graphviz_GraphElement.cpp -- GraphVizGraphElement class implementation.
 */
#include "graphviz.h"

namespace otawa { namespace display {


/**
 * @author G. Cavaignac
 * @class GraphVizGraphElement
 * This class is the superclass of elements contained in the graph (Node & Edge)
 */



bool GraphVizGraphElement::printAttribute(elm::io::Output &out, const PropList::Iter& prop){
	if(prop == STYLE){
		out << "style=";
		switch(prop.get<int>()){
			case STYLE_FILLED:
				out << "filled";
				break;
			case STYLE_PLAIN:
				out << "solid";
				break;
			case STYLE_DASHED:
				out << "dashed";
				break;
			case STYLE_DOTTED:
				out << "dotted";
				break;
			case STYLE_BOLD:
				out << "bold";
				break;
			default:
				assert(false);
		}
		return true;
	}
	else if(prop == FONT_COLOR){
		out << "fontcolor=\"" << quoteSpecials(prop.get<CString>()) << '"';
		return true;
	}
	else if(prop == FONT_SIZE){
		out << "fontsize=" << prop.get<int>();
		return true;
	}
	else if(prop == FONT){
		out << "fontname=\"" << quoteSpecials(prop.get<CString>()) << '"';
		return true;
	}
	return GraphVizItem::printAttribute(out, prop);
}



} }




