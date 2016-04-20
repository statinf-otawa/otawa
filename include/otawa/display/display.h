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

#include <elm/io.h>
#include <elm/string.h>

namespace otawa { namespace display {

using namespace elm;

// Color class
class Color {
public:
	typedef t::uint8 comp_t;
	
	inline Color(void) { }
	Color(comp_t red, comp_t green, comp_t blue, int alpha = -1);
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

	inline ShapeStyle(void): shape(SHAPE_RECORD), raw(false) { }

	shape_t shape;
	FillStyle fill;
	LineStyle line;
	TextStyle text;
	bool raw;
};

typedef enum {
	NONE = 0,
	BOLD = 1,
	ITALIC = 2,
	UNDERLINE = 3,
	SUPER = 4,
	SUB = 5,
	TABLE = 6,
	ROW = 7,
	CELL = 8,
	TCOLOR = 9
} text_style_t;

class Tag {
public:
	inline Tag(text_style_t style, bool end): _end(end), _style(style) { }
	inline Tag(const Color& color, bool end): _end(end), _style(TCOLOR), _color(color) { }
private:
	bool _end;
	text_style_t _style;
	Color _color;
};

class Text {
public:
	virtual ~Text(void);
	virtual io::Output& out(void) = 0;
	virtual void tag(const Tag& tag) = 0;
	virtual void setURL(const string& url) = 0;
};

inline Tag begin(text_style_t style) { return Tag(style, false); }
inline Tag begin(const Color& color) { return Tag(color, false); }
inline Tag end(text_style_t style) { return Tag(style, true); }
inline Tag end(const Color& color) { return Tag(color, true); }

inline Text& operator<<(Text& out, const Tag& t) { out.tag(t); return out; }
template <class T>
inline Text& operator<<(Text& out, const T& v) { out.out() << v; return out; }

} } // otawa::display

#endif /* OTAWA_DISPLAY_DISPLAY_H */
