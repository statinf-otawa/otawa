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

#include <otawa/display/CFGDisplayer.h>
#include <otawa/program.h>

namespace otawa { namespace display {

Identifier<DisplayedCFG::Vertex> DisplayedCFG::VERTEX("");
Identifier<DisplayedCFG::Edge> DisplayedCFG::EDGE("");

/**
 * @class DisplayedCFG
 * @ref AbstractGraph implementation to display a CFG.
 * Notice it is advised to use @ref CFGDecorator to make
 * easier decoration of the CFG.
 *
 * @ingroup display
 */

/**
 */
DisplayedCFG::DisplayedCFG(CFG& cfg): _cfg(&cfg) {
	for(CFG::BlockIter b = _cfg->blocks(); b; b++) {
		VERTEX(b) = Vertex(b);
		for(Block::EdgeIter e = b->outs(); e; e++)
			EDGE(e) = Edge(e);
	}
}

/**
 */
DisplayedCFG::~DisplayedCFG(void) {
	for(CFG::BlockIter b = _cfg->blocks(); b; b++) {
		VERTEX(b).remove();
		for(Block::EdgeIter e = b->outs(); e; e++)
			EDGE(e).remove();
	}
}

class BlockIter: public dyndata::AbstractIter<const AbstractGraph::Vertex *> {
public:
	inline BlockIter(const CFG::BlockIter& iter): i(iter) { }
	virtual bool ended(void) const { return i.ended(); }
	virtual const AbstractGraph::Vertex *item(void) const { return &*DisplayedCFG::VERTEX(i); }
	virtual void next(void) { i.next(); }
private:
	CFG::BlockIter i;
};

class EdgeIter: public dyndata::AbstractIter<const AbstractGraph::Edge *> {
public:
	inline EdgeIter(const Block::EdgeIter& iter): i(iter) { }
	virtual bool ended(void) const { return i.ended(); }
	virtual const AbstractGraph::Edge *item(void) const { return &*DisplayedCFG::EDGE(i); }
	virtual void next(void) { i.next(); }
private:
	Block::EdgeIter i;
};

/**
 */
dyndata::AbstractIter<const AbstractGraph::Vertex *> *DisplayedCFG::vertices(void) const {
	return new BlockIter(_cfg->blocks());
}

/**
 */
dyndata::AbstractIter<const AbstractGraph::Edge *> *DisplayedCFG::outs(const AbstractGraph::Vertex& v) const {
	return new EdgeIter(block(v)->outs());
}

/**
 */
dyndata::AbstractIter<const AbstractGraph::Edge *> *DisplayedCFG::ins(const AbstractGraph::Vertex& v) const {
	return new EdgeIter(block(v)->ins());
}

/**
 */
const AbstractGraph::Vertex& DisplayedCFG::sourceOf(const AbstractGraph::Edge& e) const {
	return *VERTEX(edge(e)->source());
}

/**
 */
const AbstractGraph::Vertex& DisplayedCFG::sinkOf(const AbstractGraph::Edge& e) const {
	return *VERTEX(edge(e)->sink());
}

/**
 */
string DisplayedCFG::id(const AbstractGraph::Vertex& v) const {
	return _ << block(v)->id();
}


/**
 * @class CFGDecorator
 * Decorator dedicated to CFGs.
 */

CFGDecorator::CFGDecorator(WorkSpace *workspace)
:	display_assembly(true),
	display_source_line(true),
	display_props(false),
	source_color("darkgreen"),
	label_color("blue"),
	ws(workspace)
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
 */
void CFGDecorator::decorate(const AbstractGraph& graph, Text& caption, GraphStyle& style) const {
	decorate(DisplayedCFG::cfg(graph), caption, style);
}

/**
 */
void CFGDecorator::decorate(const AbstractGraph& graph, const AbstractGraph::Vertex& vertex, Text& content, VertexStyle& style) const {
	decorate(DisplayedCFG::cfg(graph), DisplayedCFG::block(vertex), content, style);
}

/**
 */
void CFGDecorator::decorate(const AbstractGraph& graph, const AbstractGraph::Edge& edge, Text& label, EdgeStyle& style) const {
	decorate(DisplayedCFG::cfg(graph), DisplayedCFG::edge(edge), label, style);
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
	if(display_assembly) {
		content << display::end(display::CELL) << display::end(display::ROW)
				<< display::hr
				<< display::begin(display::ROW) << display::begin(display::CELL);
		displayAssembly(g, b, content);
	}
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
	cstring file;
	int line = 0;
	for(BasicBlock::InstIter i = block->insts(); i; i++) {

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
		for(Identifier<Symbol *>::Getter l(i, SYMBOL); l; l++)
			content << display::begin(label_color) << l->name() << ":" << display::end(label_color)
					<< display::left;

		// display instruction
		content << ot::address(i->address()) << "  " << *i
				<< display::left;
	}
}

/**
 * Called to display the properties.
 */
void CFGDecorator::displayProps(CFG *g, BasicBlock *b, Text& content) const {
	for(PropList::Iter p(b); p; p++)
		if(p->id()->name())
			content << display::begin(display::BOLD) << p->id()->name() << display::end(display::BOLD)
					<< "\t" << *p << display::left;
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
