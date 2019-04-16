/*
 *	display classes implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
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
 * @param alpha	Alpha component.
 */
Color::Color(comp_t red, comp_t green, comp_t blue, int alpha) {
	StringBuffer b;
	b << '#' << hex2(red) << hex2(green) << hex2(blue);
	if(alpha > 0)
		b << hex2(alpha);
	text = b.toString();
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
 * @class Begin<T> display::begin(const T& v);
 * Produce a special tag that, send to a Text, starts a new style area.
 * The style depends on the passed value that may be a Color or
 * of type text_style_t.
 * @param v		Value embedded in the tag.
 * @ingroup display
 */


/**
 * @class Begin<T> display::end(const T& v);
 * Produce a special tag that, send to a Text, ends a style area.
 * The style depends on the passed value that may be a Color or
 * of type text_style_t.
 * @param v		Value embedded in the tag.
 * @ingroup display
 */


/**
 * @class Indent display::indent(int n);
 * Produce a special tag that, send to a Text, indent the display
 * of the given indentation size.
 * @param n		Indentation size.
 * @ingroup display
 */


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


/**
 * @class Text
 * Special output class to generate the content of nodes. Support specially styling of text
 * and URL associated to a vertex.
 *
 * In addition, this class supports operator "<<" as classic @ref elm::io::Output:
 * operators working on "<<" for @ref elm::io::Output will work also for this class.
 * @ingroup display
 */


/**
 */
Text::~Text(void) {
}

/**
 * @fn io::Output& Text::out(void);
 * Get the stream to output plain text.
 * @return	Output stream.
 */

/**
 * @fn void Text::tag(const Tag& tag);
 * Output a tag to change text style or layout.
 * @param tag	Tag to set.
 */

/**
 * @fn void Text::setURL(const string& url);
 * Set the URL for the full text.
 * @param url	Full-text URL.
 */

/**
 * @fn void Text::indent(int n);
 * Indent the text for the given quantity.
 * A simpler way to perform indentation is to use the function display::indent()
 * on the text.
 * @param n	Indentation (in space characters).
 */

const Tag
	br(BR),				///< Its display on a Text breaks the current line. 										@ingroup display
	left(BR_LEFT),		///< Its display on a Text breaks the current line and starts a new line aligned to left.	@ingroup display
	center(BR_CENTER),	///< Its display on a Text breaks the current line and starts a new centered line.			@ingroup display
	right(BR_RIGHT),	///< Its display on a Text breaks the current line and starts a new line aligned to right.	@ingroup display
	hr(HR);				///< Its display on a Text breaks the current line and display an horizontal rule.			@ingroup display


/**
 * Specialize input reading of output mode for option argument decoding.
 */
string operator>>(string s, output_mode_t& mode) {
	static struct { cstring name; output_mode_t mode; } labs[]= {
		{ "ps", 	OUTPUT_PS },
		{ "pdf",	OUTPUT_PDF },
		{ "png",	OUTPUT_PNG },
		{ "gif",	OUTPUT_GIF },
		{ "jpg",	OUTPUT_JPG },
		{ "svg",	OUTPUT_SVG },
		{ "dot",	OUTPUT_DOT },
		{ "rawdot",	OUTPUT_RAW_DOT },
		{ "view",	OUTPUT_VIEW },
		{ "",		OUTPUT_ANY}
	};
	for(int i = 0; labs[i].mode != OUTPUT_ANY; i++)
		if(labs[i].name == s) {
			mode = labs[i].mode;
			return s;
		}
	throw IOException(_ << s << " is not a valid output mode!");
}

/**
 * @class class TypeOption;
 * Command line parameter implementation allowing to select a display output mode.
 * @ingroup display
 */

/**
 */
TypeOption::TypeOption(option::Manager& man)
: option::ValueOption<output_mode_t>(Make(man)
	.cmd("-T")
	.cmd("--out-type")
	.description("select the type of output.")
	.argDescription("type")
	.def(OUTPUT_ANY))
{ }


/**
 * @class OutputOption
 * Command line parameter implementation allowing to select the output path.
 * @ingroup display
 */

OutputOption::OutputOption(option::Manager& man)
:	option::ValueOption<sys::Path>(Make(man)
		.cmd("-o")
		.cmd("--output")
		.description("select the path to output to.")
		.argDescription("path"))
{ }


/**
 * @class VertexStyle
 * Describes the style for a vertex as used by otawa::display::Displayer class.
 * It is the aggregate of a shape, margin size, fill style, text style and
 * line style.
 *
 * @ingroup display
 */

/**
 * @class EdgeStyle
 * Describes the style for an edge as used by otawa::display::Displayer class.
 * It is the aggregate of a text style and a line style.
 *
 * @ingroup display
 */

} } // otawa::display

BEGIN_ENUM(otawa::display::output_mode_t)
	.value("ANY",		otawa::display::OUTPUT_ANY)
	.value("PS", 		otawa::display::OUTPUT_PS)
	.value("PDF", 		otawa::display::OUTPUT_PDF)
	.value("PNG", 		otawa::display::OUTPUT_PNG)
	.value("GIF", 		otawa::display::OUTPUT_GIF)
	.value("JPG", 		otawa::display::OUTPUT_JPG)
	.value("SVG", 		otawa::display::OUTPUT_SVG)
	.value("DOT", 		otawa::display::OUTPUT_DOT)
	.value("RAW_DOT",	otawa::display::OUTPUT_RAW_DOT)
	.value("VIEW", 		otawa::display::OUTPUT_VIEW)
END_ENUM;


otawa::display::Plugin otawa_display;
ELM_PLUGIN(otawa_display, OTAWA_PROC_HOOK);
