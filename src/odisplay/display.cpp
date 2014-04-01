/*
 *	$Id$
 *	display classes implementation
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

#include <otawa/display/display.h>
#include <otawa/proc/ProcessorPlugin.h>

using namespace elm;

namespace otawa { namespace display {

inline io::IntFormat hex2(int v) {
	return io::width(2, io::pad('0', io::hex(v)));
}

/**
 * @class Color
 * A color description.
 * @ingroup display
 */


/**
 * Build a color from its components.
 * @param red	Red component.
 * @param green	Green component.
 * @param blue	Blue component.
 */
Color::Color(comp_t red, comp_t green, comp_t blue) {
	text = _ << '#' << hex2(red) << hex2(green) << hex2(blue);
}


/**
 * Build a color from its RGB value.
 * @param color		RGB value (bits 23-16: red, bits 15-8: green, bits 7..0: blue).
 */
Color::Color(t::uint32 color) {
	text = _ << '#' << io::pad('0', io::width(6, io::hex(color)));
}


/**
 * Create a color by its name.
 * @param name	Name of the color.
 */
Color::Color(String name) {
	text = name;
}


/**
 * @class TextStyle
 * Description of the style of text.
 * @ingroup display
 */


/**
 * @var TextStyle::name
 * Name of the text font.
 */


/**
 * @var TextStyle::style
 * Style of the displayed text. Either @ref NORMAL, or an OR'ed combination
 * of @ref BOLD, @ref ITALIC or @ref UNDERLINE.
 */


/**
 * @var TextStyle::size
 * The size of the font.
 */


/**
 * @var TextStyle::color
 * Color the displayed text.
 */


/**
 * @var LineStyle::color
 * The color of the line.
 */


/**
 * @var LineStyle::weight
 * The wright of the line : 0 for minimal line width.
 */


/**
 * @var LineStyle::style
 * The style of the line : one of @ref HIDDEN, @ref PLAIN, @ref DOTTED or
 * @ref DASHED.
 */


/**
 * @class FillStyle
 * The style of a filled area.
 * @ingroup display
 */


/**
 * @var FillStyle::color;
 * Background color.
 */


/**
 * @var FillStyle::fill
 * Fill style of the area. One of @ref FILL_NONE or @ref FILL_SOLID.
 */


/**
 * @class ShapeStyle
 * Shape style.
 * @ingroup display
 */


/**
 * @var ShapeStyle::shape;
 * The used shape (one of SHAPE_xxx).
 */


/**
 * @var ShapeStyle::fill;
 * Style used to fill the shape.
 */


/**
 * @var ShapeStyle::line;
 * Style used to draw the border of the shape.
 */


/**
 * @var ShapeStyle::text;
 * Style to draw text in the shape.
 */

class Plugin: public otawa::ProcessorPlugin {
public:
	Plugin(void): ProcessorPlugin("otawa::display", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
};

} } // otawa::display

otawa::display::Plugin OTAWA_PROC_HOOK ;
