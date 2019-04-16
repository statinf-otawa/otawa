/*
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
#include <elm/rtti.h>
#include <elm/string.h>
#include <elm/sys/Path.h>
#include <elm/option/ValueOption.h>

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
	inline operator bool() const { return text; }
	
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
	
	inline LineStyle(void): style(PLAIN) { }

	Color color;
	style_t style;
};


// FillStyle class
class FillStyle {
public:
	typedef enum fill_t {
		FILL_NONE,
		FILL_SOLID
	} fill_t;
	
	inline FillStyle(void): fill(FILL_NONE) { }
	
	Color color;
	fill_t fill;
};


// VertexStyle class
class VertexStyle {
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

	inline VertexStyle(void): shape(SHAPE_RECORD), margin(-1), raw(false) { }

	shape_t shape;
	FillStyle fill;
	LineStyle line;
	TextStyle text;
	int margin;
	bool raw;
};
typedef VertexStyle ShapeStyle;

// EdgeStyle class
class EdgeStyle {
public:
	TextStyle text;
	LineStyle line;
};

// GraphStyle class
class GraphStyle {
public:
	TextStyle text;
};

typedef enum layout_t {
	MAPPED = 0,
	SPRING = 1,
	RADIAL = 2,
	CIRCULAR = 3
} layout_t;


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
	SMALL = 9,
	BIG = 10
} text_style_t;

template <class T>
class Begin {
public:
	inline Begin(const T& v): value(v) { }
	T value;
};

template <class T>
class End {
public:
	inline End(const T& v): value(v) { }
	T value;
};

template <class T> Begin<T> begin(const T& v) { return Begin<T>(v); }
template <class T> End<T> end(const T& v) { return End<T>(v); }

class Indent {
public:
	inline Indent(int _n = 1): n(_n) { }
	int n;
};
inline Indent indent(int n = 1) { return Indent(n); }

typedef enum {
	BR,
	BR_LEFT,
	BR_CENTER,
	BR_RIGHT,
	HR
} tag_t;

class Tag {
public:
	inline Tag(tag_t _tag): tag(_tag) { }
	tag_t tag;
};
extern const Tag br, left, center, right, hr;

enum struct align {
	center = 0,
	left = 1,
	right = 2
};

class Text {
public:
	virtual ~Text(void);
	virtual io::Output& out(void) = 0;
	virtual void tag(text_style_t style, bool end) = 0;
	virtual void color(const Color& color, bool end) = 0;
	virtual void setURL(const string& url) = 0;
	virtual void tag(const Tag& nl) = 0;
	virtual void align(display::align align) = 0;
	virtual void indent(int n) = 0;
};

inline Text& operator<<(Text& out, const Begin<text_style_t>& s) { out.tag(s.value, false); return out; }
inline Text& operator<<(Text& out, const End<text_style_t>& s) { out.tag(s.value, true); return out; }
inline Text& operator<<(Text& out, const Begin<Color>& c) { out.color(c.value, false); return out; }
inline Text& operator<<(Text& out, const End<Color>& c) { out.color(c.value, true); return out; }
inline Text& operator<<(Text& out, const Tag& tag) { out.tag(tag); return out; }
inline Text& operator<<(Text& out, display::align align) { out.align(align); return out; }
inline Text& operator<<(Text& out, Indent i) { out.indent(i.n); return out;}
template <class T>
inline Text& operator<<(Text& out, const T& v) { out.out() << v; return out; }

// Kind class
typedef enum output_mode_t {
	OUTPUT_ANY = 0,
	OUTPUT_PS,
	OUTPUT_PDF,
	OUTPUT_PNG,
	OUTPUT_GIF,
	OUTPUT_JPG,
	OUTPUT_SVG,
	OUTPUT_DOT,
	OUTPUT_RAW_DOT,
	OUTPUT_VIEW
} output_mode_t;
typedef output_mode_t kind_t;		// deprecated

class TypeOption: public option::ValueOption<output_mode_t> {
public:
	inline TypeOption(option::Manager *man): TypeOption(*man) { }
	TypeOption(option::Manager& man);
};

class OutputOption: public option::ValueOption<sys::Path> {
public:
	inline OutputOption(option::Manager *man): OutputOption(*man) { }
	OutputOption(option::Manager& man);
};

} } // otawa::display

DECLARE_ENUM(otawa::display::output_mode_t);

#endif /* OTAWA_DISPLAY_DISPLAY_H */
