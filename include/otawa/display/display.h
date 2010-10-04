/*
 *	$Id$
 *	display classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_DISPLAY_DISPLAY_H
#define OTAWA_DISPLAY_DISPLAY_H

#include <elm/string.h>

namespace otawa { namespace display {

using namespace elm;

// Color class
class Color {
public:
	typedef t::uint8 comp_t;
	
	inline Color(void) { }
	Color(comp_t red, comp_t green, comp_t blue);
	Color(t::uint32 color);
	Color(string name);
	inline const string& asText(void) const { return text; }
	
private:
	string text;
};


// TextStyle class
class TextStyle {
public:	
	static const int NORMAL		= 0x00;
	static const int BOLD		= 0x01;
	static const int ITALIC		= 0x02;
	static const int UNDERLINE	= 0x04;
	
	inline TextStyle(void): style(NORMAL), size(0) { }

	string name;
	int style;
	int size;
	Color color;
};


// LineStyle class
class LineStyle {
public:
	typedef enum style_t {
		HIDDEN,
		PLAIN,
		DOTTED,
		DASHED
	} style_t;
	
	inline LineStyle(void): weight(0), style(PLAIN) { }

	Color color;
	int weight;
	style_t style;
};


// FillStyle class
class FillStyle {
public:
	typedef enum fill_t {
		FILL_NONE,
		FILL_SOLID
	} fill_t;
	
	inline FillStyle(void): color(0xffffff), fill(FILL_NONE) { }
	
	Color color;
	fill_t fill;
};


// ShapeStyle class
class ShapeStyle {
public:
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

	inline ShapeStyle(void): shape(SHAPE_RECORD) { }

	shape_t shape;
	FillStyle fill;
	LineStyle line;
	TextStyle text;
};

} } // otawa::display

#endif /* OTAWA_DISPLAY_DISPLAY_H */
