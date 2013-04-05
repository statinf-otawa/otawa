/*
 *	$Id$
 *	graphviz class interface
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
#ifndef OTAWA_DISPLAY_GRAPHVIZ_H
#define OTAWA_DISPLAY_GRAPHVIZ_H

#include <otawa/display/Driver.h>

namespace otawa { namespace display {
	
// GraphViz driver
extern Driver& graphviz_driver;


// Layout property
typedef enum graphviz_layout_t {
	LAYOUT_DOT,
	LAYOUT_RADIAL,
	LAYOUT_CIRCULAR,
	LAYOUT_UNDIRECTED_NEATO,
	LAYOUT_UNDIRECTED_FDP
} graphviz_layout_t;
extern Identifier<int> GRAPHVIZ_LAYOUT;


} } // otawa::display

#endif // OTAWA_DISPLAY_GRAPHVIZ_H
