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

#include <elm/sys/ProcessBuilder.h>
#include <elm/sys/System.h>
#include <elm/xom/String.h>
#include <otawa/cfg.h>
#include <otawa/manager.h>
#include <otawa/program.h>
#include "display_MKFFDotDisplayer.h"

using namespace elm;
using namespace otawa;

namespace mkff {

void MKFFDotDecorator::displaySynthBlock(CFG *g, SynthBlock *b, display::Text& content, display::VertexStyle& style) const {
	display::CFGDecorator::displaySynthBlock(g, b, content, style);
	if(b->callee()) {
		if(!b->callee()->index())
			content.setURL("index.dot");
		else
			content.setURL(_ << b->callee()->index() << "_" << b->callee()->name() << ".dot");
	}
}

void MKFFDotDecorator::displayEndBlock(CFG *graph, Block *block, display::Text& content, display::VertexStyle& style) const {
	CFGDecorator::displayEndBlock(graph, block, content, style);
	content.setURL("");
}

void MKFFDotDecorator::displayBasicBlock(CFG *graph, BasicBlock *block, display::Text& content, display::VertexStyle& style) const {
	CFGDecorator::displayBasicBlock(graph, block, content, style);
	content.setURL("");
}

void MKFFDotDecorator::displayAssembly(CFG *graph, BasicBlock *block, display::Text& content) const {
	cstring file;
	int line = 0;

	for(BasicBlock::InstIter i = block->insts(); i(); i++) {
		// display source line
		if(display_source_line) {
			Option<Pair<cstring, int> > src = workspace()->process()->getSourceLine(i->address());
			if(src && ((*src).fst != file || (*src).snd != line)) {
				file = (*src).fst;
				line = (*src).snd;
				content << display::begin(source_color) << display::begin(display::ITALIC) << file << ":" << line << display::end(display::ITALIC) << display::end(source_color)
						<< display::left;
			}
		}
		// display labels
		for(Identifier<Symbol *>::Getter l(*i, SYMBOL); l(); l++)
			content << display::begin(label_color) << l->name() << ":" << display::end(label_color)
					<< display::left;
		// adding the color indicating the instruction is sliced away
		if(randomColors)
			content << display::begin(display::Color(sys::System::random(255), sys::System::random(255), sys::System::random(255)));
		// display instruction
		content << ot::address(i->address()) << "  " << *i;
		// end adding the color indicating the instruction is sliced away
		if(randomColors)
			content << display::end(display::Color(sys::System::random(255), sys::System::random(255), sys::System::random(255)));

		content << display::left;
	} // end for each instructions
}

} // namespace mkff
