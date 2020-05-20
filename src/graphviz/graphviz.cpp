/*
 *	graphviz plugin
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-18, IRIT UPS.
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

#include "config.h"
#include <elm/sys/ProcessBuilder.h>
#include <elm/sys/System.h>
#include <otawa/display/Displayer.h>
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa { namespace graphviz {

using namespace otawa;

class Text;

// Filter for HTML characters
class HTMLFilter: public io::OutStream {
public:
	HTMLFilter(io::OutStream& output, Text& text): out(output), _text(text) { }

	virtual int write(const char *buffer, int size) {
		for(const char *p = buffer; p < buffer + size; p++) {
			int r;
			switch(*p) {
			case '<':	r = out.write("&lt;", 4); 	break;
			case '>':	r = out.write("&gt;", 4);	break;
			case '&':	r = out.write("&amp;", 5);	break;
			case '"':	r = out.write("&quot;", 6);	break;
			case '{':	r = out.write("\\{", 2);	break;
			case '}':	r = out.write("\\}", 2);	break;
			case '|':	r = out.write("\\|", 2);	break;
			case '\n':	newLine();					break;
			default:	r = out.write(*p);			break;
			}
			if(r < 0)
				return r;
		}
		return size;
	}

	virtual int flush(void) { return out.flush(); }
	inline void newLine(void);

private:
	io::OutStream& out;
	Text& _text;
};

// Text builder with HTML tags
class Text: public display::Text {
public:

	Text(void): filter(_buf.stream(), *this), _out(filter) { reset(); }

	void reset(void) {
		accept_hr = false;
		_align = display::align::center;
	}

	io::Output& out(void) override {
		return _out;
	}

	void tag(display::text_style_t style, bool end) override {
		cstring name;

		// default: do not accept <hr/>
		accept_hr = false;

		// process each style
		switch(style) {

		// simple styles
		case display::NONE:			break;
		case display::BOLD:			name = "B"; goto gen;
		case display::ITALIC:		name = "I"; goto gen;
		case display::UNDERLINE:	name = "U"; goto gen;
		case display::SUPER:		name = "SUP"; goto gen;
		case display::SUB:			name = "SUB"; goto gen;
		case display::ROW:			name = "TR"; accept_hr = end; goto gen;
		gen:
			_buf << '<';
			if(end) _buf << "/";
			_buf << name << '>';
			break;

		// table cell
		case display::CELL:
			if(!end) {
				_buf << "<TD";
				switch(_align) {
				case display::align::center:
					break;
				case display::align::left:
					_buf << " ALIGN=\"LEFT\"";
					break;
				case display::align::right:
					_buf << " ALIGN=\"RIGHT\"";
					break;
				}
				_buf << ">";
			}
			else
				_buf << "</TD>";
			break;

		// table style
		case display::TABLE:
			if(!end) {
				_buf << "<TABLE BORDER=\"0\">";
				accept_hr = true;
			}
			else
				_buf << "</TABLE>";
			break;

		// font based style
		case display::SMALL:
			if(!end)
				_buf << "<FONT POINT-SIZE=\"10\">";
			else
				_buf << "</FONT>";
			break;
		case display::BIG:
			if(!end)
				_buf << "<FONT POINT-SIZE=\"18\">";
			else
				_buf << "</FONT>";
			break;
		default:
			break;
		}
	}

	void color(const display::Color& color, bool end) override {
		if(!end)
			_buf << "<FONT COLOR=\"" << color.asText() << "\">";
		else
			_buf << "</FONT>";
	}

	void tag(const display::Tag& tag) override {
		switch(tag.tag) {
		case display::BR:
			_buf << "<BR";
			switch(_align) {
			case display::align::center: break;
			case display::align::left: _buf << " ALIGN=\"left\""; break;
			case display::align::right: _buf << " ALIGN=\"right\""; break;
			}
			_buf << "/>";
			break;
		case display::BR_LEFT:
			_buf << "<BR ALIGN=\"left\"/>";
			break;
		case display::BR_CENTER:
			_buf << "<BR ALIGN=\"center\"/>";
			break;
		case display::BR_RIGHT:
			_buf << "<BR ALIGN=\"right\"/>";
			break;
		case display::HR:
			if(accept_hr)
				_buf << "<HR/>";
			break;
		}
	}

	void align(display::align align) override {
		_align = align;
	}

	void setURL(const string& url) override { _url = url; }

	void indent(int n) override {
		for(int i = 0; i < n; i++)
			_buf << "&nbsp;";
	}

	inline string url(void) const { return _url; }
	inline string text(void) { string r = _buf.toString(); _buf.reset(); return r; }

private:
	StringBuffer _buf;
	HTMLFilter filter;
	Output _out;
	string _url;
	bool accept_hr;
	display::align _align;
};



void HTMLFilter::newLine(void) {
	_text.tag(display::br);
	flush();
}


class Displayer: public display::Displayer {
public:

	Displayer(graph::DiGraph *g, const display::Decorator& d, display::output_mode_t mode)
		: display::Displayer(g, d, mode == display::OUTPUT_ANY ? display::OUTPUT_RAW_DOT : mode)
		{ }

	virtual void process(void) {

		// open the file
		if(_output == display::OUTPUT_RAW_DOT) {
			if(!_path)
				gen(cout);
			else {
				try {
					io::OutStream *file = sys::System::createFile(_path);
					io::Output out(*file);
					gen(out);
					delete file;
				}
				catch(sys::SystemException& e) {
					throw display::Exception(e.message());
				}
			}
		}

		// other output
		else {
			elm::sys::ProcessBuilder builder(command());
			builder.addArgument(outputOption());
			if(_path) {
				builder.addArgument("-o");
				builder.addArgument(_path.asSysString());
			}
			Pair<sys::SystemInStream *, sys::SystemOutStream *> pipe = sys::System::pipe();
			builder.setInput(pipe.fst);
			sys::Process *proc = builder.run();
			Output out(*pipe.snd);
			gen(out);
			delete pipe.fst;
			delete pipe.snd;
			proc->wait();
			if(proc->returnCode())
				throw display::Exception(_ << "failure of dot call: " << proc->returnCode());
			delete proc;
		}
	}

private:

	void gen(Output& out){
		out << "digraph main {\n";

		// generate graph style
		display::GraphStyle style;
		d.decorate(g, text, style);
		string title = text.text();
		if(title) {
			out << "\tgraph [label=<" << title << ">";
			bool com = true;
			gen(out, style.text, com);
			out << "];\n";
		}

		// generate default node style
		out << "\tnode [";
		bool com = false;
		gen(out, default_vertex.fill, com);
		gen(out, default_vertex.text, com);
		gen(out, default_vertex.shape, com);
		gen(out, default_vertex.line, com);
		genMargin(out, default_vertex, com);
		out << "];\n";

		// generate default edge style
		out << "\tedge [";
		com = false;
		gen(out, default_edge.text, com);
		gen(out, default_edge.line, com);
		out << "];\n";

		// generate the nodes
		for(auto v: *g)
			gen(out, v);

		// generate the edges
		for(auto v: *g)
			for(auto e: v->outEdges())
				gen(out, e);

		out << "}\n";
	}

	void genMargin(Output& out, const display::VertexStyle& style, bool& com) {
		if(style.margin >= 0) {
			comma(out, com);
			out << "margin=" << style.margin;
		}
	}

	void gen(Output& out, graph::Vertex *v) {
		display::VertexStyle style;
		text.reset();
		d.decorate(g, v, text, style);
		string content = text.text();
		bool com = false;
		out << '\t' << v->index() << " [";
		if(content) {
			comma(out, com);
			out << "label=<" << content << ">";
		}
		if(text.url()) {
			comma(out, com);
			out << "URL=\"" << text.url() << "\"";
		}
		gen(out, style.text, com);
		gen(out, style.fill, com);
		gen(out, style.shape, com);
		gen(out, style.line, com);
		genMargin(out, style, com);
		out << "];\n";
	}

	void gen(Output& out, graph::Edge *e) {
		display::EdgeStyle style;
		text.reset();
		d.decorate(g, e, text, style);
		string label = text.text();
		bool com = false;
		out << '\t' << e->source()->index() << " -> " << e->sink()->index() << " [";
		if(label) {
			comma(out, com);
			out << "label=<" << label << ">";
		}
		if(text.url()) {
			comma(out, com);
			out << "URL=\"" << text.url() << "\"";
		}
		gen(out, style.text, com);
		gen(out, style.line, com);
		out << "];\n";
	}

	void gen(Output& out, const display::TextStyle& text, bool& com) {
		// text.style unsupported by dot
		if(text.color.asText()) {
			comma(out, com);
			out << "color = \"" << text.color.asText() << "\"";
		}
	}

	void gen(Output& out, display::ShapeStyle::shape_t shape, bool& com) {
		static cstring names[] = {
			"",
			"record",
			"Mrecord",
			"box",
			"circle",
			"ellipse",
			"egg",
			"triangle",
			"trapezium",
			"parallelogram",
			"hexagon",
			"octagon",
			"diamond"
		};
		if(shape == display::ShapeStyle::SHAPE_NONE
		|| shape >  display::ShapeStyle::SHAPE_DIAMOND
		|| shape == display::ShapeStyle::SHAPE_RECORD)
			return;
		comma(out, com);
		out << "shape=" << names[shape] << "";
	}

	void gen(Output& out, const display::FillStyle& fill, bool& com) {
		if(fill.color.asText()) {
			comma(out, com);
			out << "fillcolor=\"" << fill.color.asText() << "\"";
		}
		if(fill.fill != display::FillStyle::FILL_NONE) {
			static cstring names[] = {
				"",
				"solid"
			};
			if(fill.fill <= display::FillStyle::FILL_SOLID) {
				comma(out, com);
				out << "style=\"" << names[fill.fill] << "\"";
			}
		}
	}

	void gen(Output& out, const display::LineStyle& line, bool& com) {
		if(line.style != display::LineStyle::PLAIN
		&& line.style <= display::LineStyle::DASHED) {
			static cstring names[] = {
				"invis",
				"plain",
				"dotted",
				"dashed"
			};
			comma(out, com);
			out << "style=\"" << names[line.style] << "\"";
		}
		if(line.color.asText()) {
			comma(out, com);
			out << "color=\"" << line.color.asText() << "\"";
		}
	}

	void comma(Output& out, bool& com) {
		if(com)
			out << ", ";
		else
			com = true;
	}

	cstring command(void) {
		switch(_layout) {
		case display::RADIAL:	return "twopi";
		case display::CIRCULAR:	return "circo";
		case display::SPRING:	return "neato";
		case display::MAPPED:
		default:				return "dot";
		}
	}

	cstring outputOption(void) {
		switch(_output) {
		case display::OUTPUT_DOT:	return "-Tdot";
		case display::OUTPUT_PDF:
		case display::OUTPUT_PS:
		case display::OUTPUT_ANY:	return "-Tps";
		case display::OUTPUT_PNG:	return "-Tpng";
		case display::OUTPUT_GIF:	return "-Tgif";
		case display::OUTPUT_JPG:	return "-Tjpg";
		case display::OUTPUT_SVG:	return "-Tsvg";
		default: ASSERT(false); return "";
		}
	}

	Text text;
};

class Provider: public display::Provider {
public:

	Provider(void): display::Provider("otawa::graphviz::Provider") { }

	/**
	 */
	virtual bool accepts(display::output_mode_t out) {
		switch(out) {
		case display::OUTPUT_ANY:
		case display::OUTPUT_PS:
		case display::OUTPUT_PDF:
		case display::OUTPUT_PNG:
		case display::OUTPUT_GIF:
		case display::OUTPUT_JPG:
		case display::OUTPUT_SVG:
		case display::OUTPUT_DOT:
		case display::OUTPUT_RAW_DOT:
#		if defined(XDOT_ENABLED) or defined(SYSTEM_VIEW_ENABLED)
			case display::OUTPUT_VIEW:
#		endif
			return true;
		default:
			return false;
		}
	}

	/**
	 */
	virtual display::Displayer *make(graph::DiGraph *g, const display::Decorator& d, display::output_mode_t out) {
		return new Displayer(g, d, out);
	}


} provider;

class Plugin: public ProcessorPlugin {
public:
	Plugin(void): ProcessorPlugin("otawa::graphviz", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
};

} }	// otawa::graphviz

otawa::graphviz::Plugin otawa_graphviz_plugin;
ELM_PLUGIN(otawa_graphviz_plugin, OTAWA_PROC_HOOK);
