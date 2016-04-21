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
#include <otawa/display/CFGDisplayer.h>
#include <otawa/program.h>

using namespace elm;
using namespace otawa;

class MultipleDotDecorator: public display::CFGDecorator {
public:
	MultipleDotDecorator(WorkSpace *ws): display::CFGDecorator(ws) { }
protected:
	virtual void displaySynthBlock(CFG *g, SynthBlock *b, display::Text& content, display::VertexStyle& style) const {
		display::CFGDecorator::displaySynthBlock(g, b, content, style);
		if(b->callee()) {
			if(!b->callee()->index())
				content.setURL("index.dot");
			else
				content.setURL(_ << b->callee()->index() << ".dot");
		}
	}

};

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

	// configuration of decorator
	MultipleDotDecorator decor(ws);
	decor.display_source_line = source_info;
	decor.display_assembly = display_assembly;

	// get the provider
	display::Provider *prov = display::Provider::get(display::OUTPUT_RAW_DOT);
	if(!prov)
		throw ProcessorException(*this, "no provider of dot output");

	for(CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		display::DisplayedCFG dcfg(**cfg);
		display::Displayer *disp = prov->make(dcfg, decor);
		if(cfg->index() == 0)
			disp->setPath(dir / "index.dot");
		else
			disp->setPath(dir / string(_ << cfg->index() << ".dot"));
		disp->process();
		delete disp;
	}
}
