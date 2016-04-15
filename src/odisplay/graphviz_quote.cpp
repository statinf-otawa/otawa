/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/graphviz_quote.cpp -- functions for escaping specials chars in strings.
 */
#include "graphviz.h"

using namespace elm;

namespace otawa { namespace display {


/**
 * This function escapes the newlines in order to
 * avoid giving the newline to DOT input
 */
String quoteNewlines(String str){
	StringBuffer buf;
	for(int i = 0; i < str.length(); i++){
		char c = str[i];
		if(c == '\n')
			buf << "\\n";
		else
			buf << c;
	}
	return buf.toString();
}



/**
 * This function escapes all the special characters,
 * and replaces the newlines by \\l (left-align in dot)
 */
String quoteSpecials(String str, bool html){
	StringBuffer buf;
	for(int i = 0; i < str.length(); i++)
		switch(str[i]) {
		case '\n':
			buf << "\\l";
			break;
		case '<':
		case '>':
			if(html) {
				buf << str[i];
				break;
			}
		case '{':
		case '}':
		case '|':
		case  '\\':
		case '"':
			buf << '\\';
			/* no break */
		default:
			buf << str[i];
		}
	return buf.toString();
}


} }

