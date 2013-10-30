/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/graphviz_Item.cpp -- GraphVizItem class implementation.
 */
#include "graphviz.h"

namespace otawa { namespace display {



/**
 * @author G. Cavaignac
 * @class GraphVizItem
 * This class is the superclass of all graphviz elements (Node, Edge & Graph)
 */



/**
 * Used to set the default behaviour of properties listing to "INCLUDE"
 */
GraphVizItem::GraphVizItem():_defaultInclude(true){}




/**
 * Makes a string containing the values of all properties in _props
 * separated by newlines
 * @return string containing properties values
 */
String GraphVizItem::getPropertiesString(){
	StringBuffer buf;
	for(PropList::Iter prop(_props); !prop.ended(); prop.next()){
		const AbstractIdentifier *id = prop->id();
		if((_defaultInclude && !_exclude.exists(id)) || _include.exists(id)){
			if(buf.length() != 0){
				buf << '\n';
			}
			prop->print(buf);
		}
	}
	return buf.toString();
}



/**
 * Prints attributes after all the style PropList have been iterated.
 * One can append endlines to the output
 * @param out output where to print others attributes
 */
void GraphVizItem::printOthersAttributes(elm::io::Output& out){
}



/**
 * Process the Property given by the iterator pointing to it.
 * Prints the value to the given output, or store the property for example
 * (the property can be printed furthermore by printOthersAttributes())
 * @param out output where to print when something have to be printed
 * @prop iterator on properties.
 * Used to compare the identifier and to get the property value
 * @return true if something have been written on the output, false else
 */
bool GraphVizItem::printAttribute(elm::io::Output &out, const PropList::Iter& prop){
	if(prop == DEFAULT){
		AbstractIdentifier *id = INCLUDE(prop);
		ASSERT(id == &INCLUDE || id == &EXCLUDE);
		if(id == &INCLUDE)
			_defaultInclude = true;
		else if(id == &EXCLUDE)
			_defaultInclude = false;
	}
	else if(prop == INCLUDE){
		_include.put(INCLUDE(prop),0);
		return false;
	}
	else if(prop == EXCLUDE){
		_exclude.put(EXCLUDE(prop),0);
		return false;
	}
	else if(prop == COLOR){
		out << "color=\"" << quoteSpecials(COLOR(prop)) << '"';
		return true;
	}
	if(prop == BACKGROUND){
		out << "bgcolor=\"" << quoteSpecials(BACKGROUND(prop)) << '"';
		return true;
	}
	else if(prop == HREF){
		out << "href=\"" << quoteSpecials(HREF(prop)) << '"';
		return true;
	}
	return false;
}


/**
 * Prints all attributes (styles) in a string, between brackets.
 * First iterates on the style PropList, calling printAttribute() for
 * each property, and then, calls printOthersAttributes() to finish.
 * This functions escapes all the newlines that have been
 * appened in the functions printAttribute() and printOthersAttributes()
 * @param style styles to be printed
 * @return String containing the attributes.
 * For example: [color="red"]
 */
String GraphVizItem::attributes(const PropList& style){
	elm::StringBuffer buf;
	buf << '[';
	for(PropList::Iter iter(style); !iter.ended(); iter.next()){
		if(printAttribute(buf, iter)){
			buf << ',';
		}
	}
	printOthersAttributes(buf);
	buf << ']';
	if(buf.length() <= 2)
		return ""; // No attributes
	String str = quoteNewlines(buf.toString());
	return str;
}


/**
 * @see Item::setProps(const PropList& props)
 */
void GraphVizItem::setProps(const PropList& props){
	_props.clearProps();
	_props.addProps(props);
}

/**
 * @fn GraphVizItem::attributes(void)
 * calls attributes(const PropList& style) with the PropList containing
 * the style of the item. Generally, the proplist is the object itself,
 * because it inherits from PropList
 */



} }


