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
#include "display_MultipleDotDisplayer.h"

using namespace elm;
using namespace otawa;

namespace mkff {

void MultipleDotDecorator::displaySynthBlock(CFG *g, SynthBlock *b, display::Text& content, display::VertexStyle& style) const {
	display::CFGDecorator::displaySynthBlock(g, b, content, style);
	if(b->callee()) {
		if(!b->callee()->index())
			content.setURL("index.dot");
		else
			content.setURL(_ << b->callee()->index() << "_" << b->callee()->name() << ".dot");
	}
}

void MultipleDotDecorator::displayEndBlock(CFG *graph, Block *block, display::Text& content, display::VertexStyle& style) const {
	CFGDecorator::displayEndBlock(graph, block, content, style);
	content.setURL("");
}

void MultipleDotDecorator::displayBasicBlock(CFG *graph, BasicBlock *block, display::Text& content, display::VertexStyle& style) const {
	CFGDecorator::displayBasicBlock(graph, block, content, style);
	content.setURL("");
}

/**
 */
void MultipleDotDecorator::decorate(CFG *graph, otawa::Edge *edge, display::Text& label, display::EdgeStyle& style) const {
	if(!edge->source()->isBasic()
	|| !edge->sink()->isBasic())
		style.line.style = display::LineStyle::DASHED;
	else if(edge->isTaken())
		label << "taken";
	else if(edge->isBoth())
		label << "both";
}

void MultipleDotDecorator::displayProps(CFG *g, BasicBlock *b, display::Text& content) const {
	for(PropList::Iter p(b); p(); p++)
		if(p->id()->name())
			content << display::begin(display::BOLD) << p->id()->name() << display::begin(display::BOLD)
					<< "\t!!!!!!" << *p;
}

} // namespace mkff
