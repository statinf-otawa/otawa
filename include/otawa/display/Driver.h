/*
 *	$Id$
 *	Driver class interface
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
#ifndef OTAWA_DISPLAY_DRIVER_H
#define OTAWA_DISPLAY_DRIVER_H

#include <otawa/display/Graph.h>

namespace otawa { namespace display {

using namespace elm;


// Kind class
typedef enum kind_t {
	OUTPUT_ANY = 0,
	OUTPUT_PS,
	OUTPUT_PDF,
	OUTPUT_PNG,
	OUTPUT_GIF,
	OUTPUT_JPG,
	OUTPUT_SVG,
	OUTPUT_DOT,
	OUTPUT_RAW_DOT
} kind_t;


// Driver class
class Driver {
public:
	
	static Driver *find(string name = "");
	static Driver *find(kind_t kind);
	
	virtual ~Driver(void) { }
	virtual Graph *newGraph(
		const PropList& defaultGraphStyle = PropList::EMPTY,
		const PropList& defaultNodeStyle = PropList::EMPTY,
		const PropList& defaultEdgeStyle = PropList::EMPTY) const = 0;
};


// Exception class
class Exception: public otawa::Exception {
public:
	inline Exception(const String& message)
		: otawa::Exception(message) { }
};


// Configuration
extern Identifier<kind_t> OUTPUT_KIND;
extern Identifier<string> OUTPUT_PATH;

} } // otawa::display

#endif // OTAWA_DISPLAY_DRIVER_H
