/*
 *	MultipleDotDisplayer implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include "MultipleDotDisplayer.h"

#include <elm/sys/System.h>
#include <elm/xom/String.h>
#include <otawa/cfg.h>
#include <otawa/display/CFGAdapter.h>
#include <otawa/display/GenDrawer.h>
#include <otawa/program.h>

using namespace elm;
using namespace otawa;

class Escape {
public:
	typedef string t;
	static void print(io::Output& out, const string& s) { xom::String(&s).escape(out); }
};

class MultipleDecorator {
public:

	static void decorate(const display::CFGAdapter& g, Output &caption, display::TextStyle &text, display::FillStyle &fill) {
		caption << g.cfg->label() << " function";
	}

	static void decorate(const display::CFGAdapter& g, const display::CFGAdapter::Vertex& v, Output &content, display::ShapeStyle &style) {
		style.shape = display::ShapeStyle::SHAPE_MRECORD;
		if(v.b->isBasic()) {
			if(html)
				content << "<B>" << io::Tag<Escape>(_ << v.b) << "</B>";
			else
				content << v.b << " (" << v.b->address() << ")";
			if(display_assembly) {
				content << "\n---\n";
				BasicBlock *bb = v.b->toBasic();
				cstring file;
				int line = 0;
				for(BasicBlock::InstIter i = bb->insts(); i; i++) {

					// display source line
					if(source_info) {
						Option<Pair<cstring, int> > src = proc->getSourceLine(i->address());
						if(src && ((*src).fst != file || (*src).snd != line)) {
							file = (*src).fst;
							line = (*src).snd;
							if(html)
								content << "<FONT COLOR=\"green\">" << io::Tag<Escape>(file) << ":" << line << "</FONT>" << io::endl;
							else
								content << file << ":" << line << io::endl;
						}
					}

					// display labels
					for(Identifier<Symbol *>::Getter l(i, SYMBOL); l; l++) {
						if(html)
							content << "<FONT COLOR =\"blue\">" << l->name() << ":" << "</FONT>" << "\n";
						else
							content << l->name() << ":" << "\n";
					}

					// display instruction
					content << ot::address(i->address()) << "  " << *i << io::endl;
				}
			}
		}
		else
			content << v.b;
	}

	static void decorate(const display::CFGAdapter& graph, const display::CFGAdapter::Edge& e, Output &label, display::TextStyle &text, display::LineStyle &line) {
		if(e.edge->source()->isEntry()
		|| e.edge->sink()->isExit()
		|| e.edge->source()->isSynth()
		|| e.edge->sink()->isSynth())
			line.style = display::LineStyle::DASHED;
		else if(e.edge->isTaken())
			label << "taken";
	}

	static Process *proc;
	static bool display_assembly;
	static bool source_info;
	static bool html;
};

Process *MultipleDecorator::proc;
bool MultipleDecorator::display_assembly;
bool MultipleDecorator::source_info;
bool MultipleDecorator::html;


/**
 */
MultipleDotDisplayer::MultipleDotDisplayer(void): Displayer("MultipleDotDisplayer", Version(1, 0, 0)) {
}

/**
 */
void MultipleDotDisplayer::setup(WorkSpace *ws) {

	// determine directory
	if(out)
		dir = out;
	else
		dir = sys::Path(ws->process()->program()->name()).basePart();

	// build it or use it
	if(!dir.exists())
		sys::System::makeDir(dir);
}

/**
 */
void MultipleDotDisplayer::processWorkSpace(WorkSpace *ws) {
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(ws);


	for(CFGCollection::Iterator cfg(coll); cfg; cfg++) {

		// configure decorator
		MultipleDecorator::source_info = source_info;
		MultipleDecorator::display_assembly = display_assembly;
		MultipleDecorator::html = true;
		MultipleDecorator::proc = ws->process();

		// configure the drawer
		display::CFGAdapter cfga(cfg, ws);
		display::GenDrawer<display::CFGAdapter, MultipleDecorator> drawer(cfga);
		if(cfg->index() == 0)
			drawer.path = dir / "index.dot";
		else
			drawer.path = dir / string(_ << cfg->index() << ".dot");
		drawer.html = true;
		drawer.default_vertex.shape = display::ShapeStyle::SHAPE_MRECORD;

		// perform the draw
		drawer.kind = display::OUTPUT_RAW_DOT;
		drawer.draw();
	}
}
