/*
 *	display::CFGDisplayer class implementation
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

#include <otawa/program.h>
#include "../../include/otawa/display/CFGDecorator.h"

namespace otawa { namespace display {

/**
 * @class CFGDecorator
 * Decorator dedicated to CFGs.
 *
 * @ingroup display
 */

CFGDecorator::CFGDecorator(WorkSpace *workspace)
:	display_assembly(true),
	display_source_line(true),
	display_props(false),
	source_color("darkgreen"),
	label_color("blue"),
	ws(workspace),
	line(0)
{ }

/**
 */
void CFGDecorator::decorate(CFG *g, Text& caption, GraphStyle& style) const {
	caption << g->label() << " function";
}

/**
 */
void CFGDecorator::decorate(CFG *g, Block *b, Text& content, VertexStyle& style) const {
	if(b->isBasic())
		displayBasicBlock(g, b->toBasic(), content, style);
	else if(b->isSynth())
		displaySynthBlock(g, b->toSynth(), content, style);
	else
		displayEndBlock(g, b, content, style);
}

/**
 */
void CFGDecorator::decorate(CFG *graph, otawa::Edge *edge, Text& label, EdgeStyle& style) const {
	if(!edge->source()->isBasic()
	|| !edge->sink()->isBasic())
		style.line.style = display::LineStyle::DASHED;
	else if(edge->isTaken())
		label << "taken";
	else if(edge->isBoth())
		label << "both";
}

/**
 * Called to display an end block (entry, exit, unknown).
 */
void CFGDecorator::displayEndBlock(CFG *graph, Block *block, Text& content, VertexStyle& style) const {
	style.shape = display::VertexStyle::SHAPE_MRECORD;
	content << block;
}

/**
 * Called to display a synthetic block.
 */
void CFGDecorator::displaySynthBlock(CFG *graph, SynthBlock *block, Text& content, VertexStyle& style) const {
	style.shape = display::VertexStyle::SHAPE_BOX;
	content << display::begin(display::BOLD) << "BB " << block->index() << display::end(display::BOLD);
	if(block->callee())
		content << " (" << block->callee()->label() << ")";
	else
		content << "(unknown)";
}
/**
 * Called to display a basic block. It first performs a call to
 * @ref displayHeader() and, if display_assembly or display_props is set,
 * performs a call to displayBody(). Both calls are performed inside
 * a table cell (if needed).
 */
void CFGDecorator::displayBasicBlock(CFG *g, BasicBlock *b, Text& content, VertexStyle& style) const {
	style.shape = display::VertexStyle::SHAPE_MRECORD;
	style.margin = 0;
	bool table = display_assembly || display_props;
	if(table)
		content << display::begin(display::TABLE) << display::begin(display::ROW) << display::begin(display::CELL);
	displayHeader(g, b, content);
	if(table) {
		content << display::end(display::CELL) << display::end(display::ROW)
				<< display::hr << display::align::left
				<< display::begin(display::ROW) << display::begin(display::CELL);
		displayBody(g, b, content);
		content << display::end(display::CELL) << display::end(display::ROW) << display::end(display::TABLE);
	}
}

/**
 * Called to display the header of a basic block.
 */
void CFGDecorator::displayHeader(CFG *graph, BasicBlock *block, Text& content) const {
	content << display::begin(display::BOLD) << "BB " << block->index() << display::end(display::BOLD)
			<< " (" << block->address() << ")";
}

/**
 * Called to display the body of the basic block vertex.
 * Basically, performs a call to @ref displayAssembly() then to
 * @ref displayProps(). It handles the separation into two cells
 * of the table.
 */
void CFGDecorator::displayBody(CFG *g, BasicBlock *b, Text& content) const {
	if(display_assembly)
		displayAssembly(g, b, content);
	if(display_props) {
		content << display::end(display::CELL) << display::end(display::ROW)
				<< display::hr
				<< display::begin(display::ROW) << display::begin(display::CELL);
		displayProps(g, b, content);
	}
}

/**
 * Called to perform the display of the assembly part.
 */
void CFGDecorator::displayAssembly(CFG *graph, BasicBlock *block, Text& content) const {
	file = "";
	line = 0;
	for(BasicBlock::InstIter i = block->insts(); i(); i++) {

		// display source line
		if(display_source_line)
			displaySourceLine(i->address(), content);

		// display labels
		displayLabels(*i, content);

		// display instruction
		content << ot::address(i->address()) << "  ";
		displayInst(*i, content);
		content << display::left;
		displayInfo(*i, content);
	}
}


/**
 * Called to display an instruction. Just output the instruction
 * as a default. Notice that this call is followed by the display
 * of a line return.
 * @param i		Instruction to display.
 * @param text	Text to output to.
 */
void CFGDecorator::displayInst(Inst *i, Text& content) const {
	content << i;
}


/**
 * This function is called to customize the output of information following
 * an instruction. It must be ended by a line return.
 * The default implementation does nothing.
 * @param i		Current instruction.
 * @param text	Text to output to.
 */
void CFGDecorator::displayInfo(Inst *i, Text& content) const {
}


/**
 * Display source line information corresponding to the given instruction.
 * @param addr	Current address.
 * @param text	Formatted stream to display to.
 */
void CFGDecorator::displaySourceLine(Address addr, Text& content) const {
	Option<Pair<cstring, int> > src = workspace()->process()->getSourceLine(addr);
	if(src && ((*src).fst != file || (*src).snd != line)) {
		file = (*src).fst;
		line = (*src).snd;
			content << display::begin(source_color) << display::begin(display::ITALIC)
					<< file << ":" << line << display::end(display::ITALIC)
					<< display::end(source_color) << display::left;
	}
}


/**
 * Display labels corresponding to the given instruction.
 * @param i			Current instruction.
 * @param content	Formatted stream to display to.
 */
void CFGDecorator::displayLabels(Inst *i, Text& content) const {
	for(Identifier<Symbol *>::Getter l(i, SYMBOL); l(); l++)
		content << display::begin(label_color) << l->name() << ":" << display::end(label_color)
				<< display::left;
}


/**
 * Called to display the properties.
 */
void CFGDecorator::displayProps(CFG *g, BasicBlock *b, Text& content) const {
	for(PropList::Iter p(b); p(); p++)
		if(p->id()->name()) {
			StringBuffer out;
			p->id()->print(out, *p);
			content << display::begin(display::BOLD) << p->id()->name() << display::end(display::BOLD)
					<< "\t" << out.toString() << display::left;
		}
}

/**
 *
 */
void CFGDecorator::setDisplayOptions(bool _da, bool _dsl, bool _dp, Color _sc, Color _lc) {
	display_assembly = _da;
	display_source_line = _dsl;
	display_props = _dp;
	source_color = _sc;
	label_color = _lc;
}

} }		// otawa::display
