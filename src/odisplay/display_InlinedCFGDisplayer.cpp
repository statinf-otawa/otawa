/*
 *	display::InlinedCFG class implementation
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

#include <otawa/display/InlinedCFGDisplayer.h>
#include <otawa/program.h>

namespace otawa { namespace display {

Identifier<InlinedCFG::Vertex> InlinedCFG::VERTEX("");
Identifier<InlinedCFG::Edge> InlinedCFG::EDGE("");

/**
 * @class InlinedCFG
 * @ref AbstractGraph implementation to display a CFG.
 * Notice it is advised to use @ref InlinedCFGDecorator to make
 * easier decoration of the CFG.
 *
 * @ingroup display
 */

/**
 */
InlinedCFG::InlinedCFG(CFG& cfg): _cfg(&cfg) {
	// mark VERTEX for each block and EDGE for each out-going edge of each block for later use
	// need to go through all the blocks because we are only processing the top level CFG
	Vector<CFG*> workList;
	workList.add(_cfg);
	while(!workList.isEmpty()) {
		CFG* cfg = workList.pop();
		for(CFG::BlockIter b = cfg->blocks(); b; b++) {
			VERTEX(b) = Vertex(b);
			if(b->isSynth() && b->toSynth()->callee()) {
				// putting the callee CFG into the work list
				workList.push(b->toSynth()->callee());
			}
			for(Block::EdgeIter e = b->outs(); e; e++)
				EDGE(e) = Edge(e);
		} // for each block
	} // for each working element
}

/**
 */
InlinedCFG::~InlinedCFG(void) {
	Vector<CFG*> workList;
	workList.add(_cfg);
	while(!workList.isEmpty()) {
		CFG* cfg = workList.pop();
		for(CFG::BlockIter b = cfg->blocks(); b; b++) {
			VERTEX(b).remove();
			if(b->isSynth() && b->toSynth()->callee()) {
				// putting the callee CFG into the work list
				workList.push(b->toSynth()->callee());
			}
			for(Block::EdgeIter e = b->outs(); e; e++)
				EDGE(e).remove();
		} // for each block
	} // for each working element
}


/**
 */
datastruct::IteratorInst<const AbstractGraph::Vertex *> *InlinedCFG::vertices(void) const {
	return new InlinedCFGBlockIter(_cfg->blocks());
}

/**
 */
datastruct::IteratorInst<const AbstractGraph::Edge *> *InlinedCFG::outs(const AbstractGraph::Vertex& v) const {
	Block* b = block(v);
	return new InlinedCFGEdgeIter(b->outs(), b);
}

/**
 */
datastruct::IteratorInst<const AbstractGraph::Edge *> *InlinedCFG::ins(const AbstractGraph::Vertex& v) const {
	return new InlinedCFGEdgeIter(block(v)->ins(), 0);
}

/**
 */
const AbstractGraph::Vertex& InlinedCFG::sourceOf(const AbstractGraph::Edge& e) const {
	// first we need to check if the edge is artificial
	InlinedCFG::Edge edgeToCheck = static_cast<const InlinedCFG::Edge&>(e);
	if(edgeToCheck.edge == 0) { // then it is artificial
		elm::cout << "artificial source " << edgeToCheck.source << io::endl;
		return *VERTEX(edgeToCheck.source);
	}
	else {
		elm::cout << "normal source " << edge(e)->source() << io::endl;
		return *VERTEX(edge(e)->source());
	}
}

/**
 */
const AbstractGraph::Vertex& InlinedCFG::sinkOf(const AbstractGraph::Edge& e) const {
	// first we need to check if the edge is artificial
	InlinedCFG::Edge edgeToCheck = static_cast<const InlinedCFG::Edge&>(e);
	if(edgeToCheck.edge == 0) { // then it is artificial
		elm::cout << "artificial target " << edgeToCheck.target << io::endl;
		return *VERTEX(edgeToCheck.target);
	}
	else {
		elm::cout << "normal target " << edge(e)->sink() << io::endl;
		return *VERTEX(edge(e)->sink());
	}
}

/**
 */
string InlinedCFG::id(const AbstractGraph::Vertex& v) const {
	return _ << block(v)->id();
}


//inline InlinedCFGBlockIter(const CFG::BlockIter& iter): i(iter) { }
bool InlinedCFG::InlinedCFGBlockIter::ended(void) const { return i.ended(); }
const AbstractGraph::Vertex *InlinedCFG::InlinedCFGBlockIter::item(void) const {
	InlinedCFG::Vertex v = VERTEX(i);
	ASSERT(v.block);
	return &*InlinedCFG::VERTEX(i);
}


void InlinedCFG::InlinedCFGBlockIter::next(void) {
	// in the inlining mode, the Synth block will not be shown, and we have to follow to the entry node of the callee
	// we use a SLList to track the stack when following and exiting on an function call
	// when reaching the last block

	if(i->cfg()->count()-1 == i->index()) {
		if (vbi.count()) { // check if it is at the end of the stack
			i = vbi.pop(); // pop the call stack, go to next block after the caller block
			// this is the case if the block obtained is also a synth block
			// i.e. BB6->BB7, both BB6 and BB7 are synth blocks. When cfg of BB6 finishes, BB7 is loaded
			// however we don't want synth block in the CFG hence we go into the synth block right away
			while (i->isSynth() && i->toSynth()->callee()) { // when entering a function
				if(!travledCFG.contains(i->toSynth()->callee())) {
					travledCFG.add(i->toSynth()->callee());
					CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
					i.next(); // consume the synth block
					vbi.push(i);
					i = j; // goes into the call
				}
				else
					i.next();
			}
			return; // return (1) the next block after the caller block, or (2) the entry block of the cfg if the block in (1) is a synth block
		}
		else { // when there is no more call stack to pop, and reaches the last block
			stop = true;
		}
	} // if reaches the last block

	i.next();

	if(stop)
		return;

	while(i->isSynth() && i->toSynth()->callee()) { // when entering a function
		if(!travledCFG.contains(i->toSynth()->callee())) {
			travledCFG.add(i->toSynth()->callee());
			CFG::BlockIter j(i->toSynth()->callee()->blocks()); // j is just for temp
			i.next(); // consume the synth block
			vbi.push(i);
			i = j;
		}
		else // ignore the SynthBlock because already traversed
			i.next();
	}
}

InlinedCFG::InlinedCFGEdgeIter::InlinedCFGEdgeIter(const Block::EdgeIter& iter, Block* b): i(iter), sourceBlock(b), currentArtificialEdge(0,0,0), moreArtificialEdges(false) {
	// scan through the out-going edges of the block
	for(Block::EdgeIter bei = b->outs(); bei; bei++) {
		// if the out-going target is a synth block, we create a link the the entry of the callee directly
		if(bei->target()->isSynth() && bei->target()->toSynth()->callee()) {
			artificialEdges.add(InlinedCFG::Edge(0, b, bei->target()->toSynth()->callee()->entry()));
			moreArtificialEdges = true;
		}
	}

	// we also need to create an artificial edge for exit to its caller
	if(b->isExit()) {
		for(CFG::CallerIter ci = b->cfg()->callers(); ci; ci++) {
			for(Block::EdgeIter ei = (*ci)->outs(); ei; ei++) {
				if(ei->target()->isSynth() && ei->target()->toSynth()->callee()) {
					artificialEdges.add(InlinedCFG::Edge(0, b, ei->target()->toSynth()->callee()->entry()));
					moreArtificialEdges = true;
				}
				else {
					artificialEdges.add(InlinedCFG::Edge(0, b, ei->target()));
					moreArtificialEdges = true;
				}
			}
		}
	}

	// need to consume the edges (at the beginning of the list) to the synth blocks, because we don't need to draw them
	while(!i.ended()) {
		InlinedCFG::Edge e = EDGE(i);
		if(e.edge->sink()->isSynth() && e.edge->sink()->toSynth()->callee()) { // if the next edge goes to a synth block, ignore
			i.next();
		}
		else
			break;
	}

	// we don't have any more regular edges, and currentArtificialEdge is not set, but we have artificialEdges available
	if(i.ended() && currentArtificialEdge.edge == 0 && currentArtificialEdge.source == 0 && currentArtificialEdge.target == 0 && artificialEdges.count() != 0) {
		currentArtificialEdge = artificialEdges.pop();
	}
}


bool InlinedCFG::InlinedCFGEdgeIter::ended(void) const {
	if(i.ended() && !moreArtificialEdges)
		return true;
	else
		return false;
}


const AbstractGraph::Edge *InlinedCFG::InlinedCFGEdgeIter::item(void) const {
	if(i.ended()) { // so we are loading the artificial edges
		return new InlinedCFG::Edge(0, currentArtificialEdge.source, currentArtificialEdge.target);
	}
	InlinedCFG::Edge e = EDGE(i);
	ASSERT(e.edge);
	return &*InlinedCFG::EDGE(i);
}


void InlinedCFG::InlinedCFGEdgeIter::next(void) {
	if(!i.ended())
		i.next();
	else if(artificialEdges.count() != 0)
		currentArtificialEdge = artificialEdges.pop();
	else
		moreArtificialEdges = false;

	if(!i.ended()) {
		InlinedCFG::Edge e = EDGE(i);
		if(e.edge->sink()->isSynth() && e.edge->sink()->toSynth()->callee()) { // if the next edge goes to a synth block, ignore
			i.next();
		}
	}
}

/**
 * @class InlinedCFGDecorator
 * Decorator dedicated to CFGs.
 */

InlinedCFGDecorator::InlinedCFGDecorator(WorkSpace *workspace)
:	display::CFGDecorator(workspace),
	ws(workspace)
{ }

/**
 */
void InlinedCFGDecorator::decorate(const AbstractGraph& graph, Text& caption, GraphStyle& style) const {
	display::CFGDecorator::decorate(InlinedCFG::cfg(graph), caption, style);
}

/**
 */
void InlinedCFGDecorator::decorate(const AbstractGraph& graph, const AbstractGraph::Vertex& vertex, Text& content, VertexStyle& style) const {
	display::CFGDecorator::decorate(InlinedCFG::cfg(graph), InlinedCFG::block(vertex), content, style);
}

/**
 */
void InlinedCFGDecorator::decorate(const AbstractGraph& graph, const AbstractGraph::Edge& edge, Text& label, EdgeStyle& style) const {
	InlinedCFG::Edge e = static_cast<const InlinedCFG::Edge&>(edge);
	if(e.edge == 0)
		decorate(InlinedCFG::cfg(graph), e.source, e.target, label, style);
	else
		display::CFGDecorator::decorate(InlinedCFG::cfg(graph), InlinedCFG::edge(edge), label, style);
}

/*
 * Draws the edges between the caller and callee
 */
void InlinedCFGDecorator::decorate(CFG *graph, otawa::Block *source, otawa::Block *target, Text& label, EdgeStyle& style) const {
	style.line.style = display::LineStyle::DASHED;
	if(target->isEntry())
		label << "call";
	else
		label << "return";
}

/**
 * Called to display an end block (entry, exit, unknown).
 */
void InlinedCFGDecorator::displayEndBlock(CFG *graph, Block *block, Text& content, VertexStyle& style) const {
	style.shape = display::VertexStyle::SHAPE_MRECORD;
	content << block;
	if(block->isExit() || block->isEntry()) {
		content.tag(display::BR);
		content << "CFG " << block->cfg()->index() << " (" << block->cfg()->name() << ")";
	}

}

/**
 * Called to display the header of a basic block.
 */
void InlinedCFGDecorator::displayHeader(CFG *graph, BasicBlock *block, Text& content) const {
	content << display::begin(display::BOLD) << "CFG " << block->cfg()->index() << " BB " << block->index() << display::end(display::BOLD)
			<< " (" << block->address() << ")";
}

} }		// otawa::display
