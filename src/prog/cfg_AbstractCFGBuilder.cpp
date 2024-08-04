/*
 *	AbstractCFGBuilder class implementation
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

#include <elm/assert.h>
#include <otawa/cfg/AbstractCFGBuilder.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/File.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>
#include "../../include/otawa/flowfact/FlowFactLoader.h"

namespace otawa {

static Identifier<int> CFG_INDEX("", -1);
static Identifier<BasicBlock *> BB("", 0);

/**
 * @class AbstractCFGBuilder
 * This class  provides common facilities
 * for processor building CFGs like @ref CFGCollector  or @ref CFGBuilder.
 *
 * @par Configuration
 * @li @ref BB_BOUNDS -- extra basic block bound.
 *
 * @ingroup cfg
 */

/**
 * Get the actual kind of an instruction
 * (taking into the @ref ALT_KIND property).
 * @param i	Instruction look in.
 * @return	Actual kind of the instruction.
 */
static inline Inst::kind_t kind(Inst *i) {
	if(i->hasProp(ALT_KIND))
		return ALT_KIND(i);
	else
		return i->kind();
}

/**
 * Test if instruction is control, instruction itself
 * or as an effect of annotations.
 * @param i		Examined instruction.
 * @return		True if it is control, false else.
 */
static bool isControl(Inst *i) {
	Inst::kind_t k = kind(i);
	return ((k & Inst::IS_CONTROL) && !IGNORE_CONTROL(i)) || IS_RETURN(i);
}


/**
 * Test if the instruction is a return.
 * @param i	Instruction to test.
 * @return	True if i is a return, false else.
 */
static bool isReturn(Inst *i) {
	Inst::kind_t k = kind(i);
	return (k & Inst::IS_RETURN) || IS_RETURN(i);
}


/**
 * Test if the instruction is a call.
 * @param i	Instruction to test.
 * @return	True if i is a call, false else.
 */
static bool isCall(Inst *i) {
	Inst::kind_t k = kind(i);
	return k & Inst::IS_CALL;
}


/**
 * Test if an instruction is marked as a block begin.
 * @param i		Instruction to look in.
 * @return		True if it is block start, false else.
 */
static bool isBlockStart(Inst *i) {
	return BB(i).exists();
}


/**
 * Test if, for the given instruction, the control can flow
 * after (conditional branch, function call).
 * @param i		Instruction to look in.
 * @return		True if flow can continue, false else.
 */
static bool canFlowAfter(Inst *i) {
	auto k = kind(i);
	if(IGNORE_SEQ(i))
		return false;
	else if(!(k & Inst::IS_CONTROL))
		return true;
	else if(k & Inst::IS_COND)
		return true;
	else if(!isCall(i))
		return false;
	else {
		Inst *t = i->target();
		if(t == nullptr)
			return true;
		else
			return !*otawa::NO_RETURN(t);
	}
}


/**
 * Get target of a non-return branch.
 * @param i		Instruction to get target from.
 * @param t		Vector store targets in.
 * @param ws	Current workspace.
 * @param id	Target identifiers.
 */
static void targets(Inst *i, Vector<Inst *>& t, WorkSpace *ws, Identifier<Address>& id) {
	if(i->target())
		t.add(i->target());
	else {
		for(Identifier<Address>::Getter a(i, id); a(); a++) {
			Inst *i = ws->findInstAt(*a);
			if(i)
				t.add(i);
		}
	}
}

static bool isNewSeqFuncStart(Inst *i, WorkSpace *ws) {
	for(auto symb : ws->process()->program()->symbols())  {
		if(symb->kind() == Symbol::FUNCTION) {
			if(i->address() == symb->address())
				return true;
		}
	}
	return false;
}

/**
 * Scan the CFG to find all BBs.
 * @param e		Entry instruction.
 * @param bbs	To store found basic blocks.
 */
void AbstractCFGBuilder::scanCFG(Inst *e, FragTable<Inst *>& bbs) {
	if(logFor(Processor::LOG_FUN))
		log << "\tscanning CFG at " << e->address() << io::endl;

	// traverse all instruction sequences until end
	Vector<Inst *> ts;
	Vector<Inst *> todo;
	todo.add(e);
	while(todo) {

		// next block
		Inst *i = todo.pop();

		// already known?
		if(isBlockStart(i))
			continue;

		// record the new block
		BB(i) = 0;
		bbs.add(i);

		// iterate until sequence end
		while(!isControl(i)) {
			Inst *n = workspace()->findInstAt(i->topAddress());
			if(!n || isBlockStart(n))
				break;
			i = n;
		}

		// push sequence if required
		if(canFlowAfter(i)) {
			Inst *n = i;
			while(!n->isBundleEnd())
				n = workspace()->findInstAt(n->topAddress());
			n = workspace()->findInstAt(n->topAddress());
			if(n && !isNewSeqFuncStart(n, workspace()))
				todo.push(n);
		}

		// delay return and call processing
		if(isReturn(i) || isCall(i))
			continue;

		// push targets
		targets(i, ts, workspace(), otawa::BRANCH_TARGET);
		todo.addAll(ts);
		ts.clear();
	}
}

/**
 * Build the required basic blocks.
 * @param bbs	Basic block entries.
 * @param maker	Current CFG maker.
 */
void AbstractCFGBuilder::buildBBs(CFGMaker& maker, const FragTable<Inst *>& bbs) {
	for(FragTable<Inst *>::Iter e(bbs); e(); e++) {
		if(NO_BLOCK(e.item())) {
			if(logFor(LOG_BLOCK))
				log << "\t\tskipping BB at " << e->address() << " (NO_BLOCK)" << io::endl;
			continue;
		}

		// build list of instructions
		Vector<Inst *> insts;
		insts.add(*e);
		if(!e->isControl())
			for(Inst *i = e->nextInst(); i && !isBlockStart(i); i = i->nextInst()) {
				insts.add(i);
				// Control instruction reached: add conditional bundle and finish
				if(isControl(i)) {
					while(!i->isBundleEnd()) {
						i = i->nextInst();
						insts.add(i);
					}
					break;
				}
			}
		else {
			// Control instruction reached: add conditional bundle and finish
			Inst *i = *e;
			while (!i->isBundleEnd()) {
				i = i->nextInst();
				insts.add(i);
			}
		}

		// create the basic block
		BasicBlock *v = new BasicBlock(insts.detach());
		maker.add(v);
		BB(*e) = v;
		if(logFor(LOG_BLOCK))
			log << "\t\tmaking BB at " << e->address() << ":" << v->topAddress() << io::endl;
	}
}

/**
 * Create a sequential edge.
 * @param m		Maker.
 * @param b		Basic block containing code.
 * @param src	Source block.
 * @param flags	Flags for the sequential edge (default to Edge::NOT_TAKEN).
 */
void AbstractCFGBuilder::seq(CFGMaker& m, BasicBlock *b, Block *src, t::uint32 flags) {
	Inst *ni = b->last()->nextInst();
	if(ni && ni->hasProp(BB)) { /*if it doesn't have the property it means we didn't visit in before, so most likely not part of this CFG*/
		// ASSERT(ni->hasProp(BB));
		m.add(src, BB(ni), new Edge(flags));
	}
}

/**
 * Build edges between basic blocks.
 * @param m	Current CFG maker.
 */
void AbstractCFGBuilder::buildEdges(CFGMaker& m) {
	Vector<Inst *> ts;
	bool first = true;
	for(CFG::BlockIter v(m.blocks()); v(); v++)

		// nothing to do with entry
		if(v->isEntry())
			continue;

		// explore BB
		else if(v->isBasic()) {

			// first block: do not forget edge with entry!
			if(first) {
				first = false;
				m.add(m.entry(), *v, new Edge(Edge::NOT_TAKEN));
			}

			// process basic block
			if(logFor(LOG_BB))
				log << "\t\tmake edge for BB " << v->address() << io::endl;
			BasicBlock *bb = **v;
			if(bb) {
				Inst *i = bb->control();

				// conditional or call case -> sequential edge (not taken)
				if(!i || (i->isConditional() && !IGNORE_SEQ(i)))
					seq(m, bb, bb);

				// branch cases
				if(i && !IGNORE_CONTROL(i)) {

					// return case
					if(isReturn(i))
						m.add(bb, m.exit(), new Edge(Edge::TAKEN));

					// not a call: build simple edges
					else if(!isCall(i)) {
						ts.clear();
						targets(i, ts, workspace(), otawa::BRANCH_TARGET);

						// no target: unresolved branch
						if(!ts)
							m.add(bb, m.unknown(), new Edge(Edge::TAKEN));

						// create edges target edges
						else
							for(Vector<Inst *>::Iter t(ts); t(); t++) {
								// clear NO_BLOCK marked targets
								if(NO_BLOCK(t.item())) {
									if(logFor(LOG_BB))
										log << "\t\tignoring NO_BLOCK at " << t->address() << io::endl;
									continue;
								}
								m.add(bb, BB(*t), new Edge(Edge::TAKEN));
							}
					}

					// a call
					else {
						ts.clear();
						targets(i, ts, workspace(), otawa::CALL_TARGET);

						// no target: unresolved call target
						if(!ts) {
							Block *b = new SynthBlock();
							m.add(b);
							m.add(bb, b, new Edge(Edge::TAKEN | Edge::CALL));
							seq(m, bb, b, Edge::NOT_TAKEN | Edge::RETURN);
						}

						// build call vertices
						else {
							bool no_return = false;
							bool one = false;
							for(Vector<Inst *>::Iter c(ts); c(); c++)
								if(NO_RETURN(*c)) {
									if(!no_return) {
										no_return = true;
										m.add(bb, m.exit(), new Edge(Edge::TAKEN | Edge::CALL));
										one = true;
									}
								}
								else if(!NO_CALL(*c)) {
									SynthBlock *cb = new SynthBlock();
									CFGMaker& cm = maker(*c);
									m.call(cb, cm);
									m.add(bb, cb, new Edge(Edge::TAKEN | Edge::CALL));
									seq(m, bb, cb, Edge::NOT_TAKEN | Edge::RETURN);
									one = true;
								}
							if(!one && !i->isConditional())
								seq(m, bb, bb, Edge::NOT_TAKEN | Edge::RETURN | Edge::CALL);
						}
					} // end else: a call

				} // end if(i && !IGNORE_CONTROL(i)) {
			}
		}
}


/**
 * Remove markers on instruction header of basic blocks.
 * @param bbs	Heads of basic block.
 */
void AbstractCFGBuilder::cleanBBs(const FragTable<Inst *>& bbs) {
	for(int i = 0; i < bbs.count(); i++)
		BB(bbs[i]).remove();
}


/**
 * Get maker for the given instruction as function entry.
 * @param i		First instruction of CFG.
 * @return		Matching maker.
 */
CFGMaker &AbstractCFGBuilder::maker(Inst *i) {
	int idx = CFG_INDEX(i);
	if(idx >= 0)
		return *makers[idx].snd;
	else {
		CFGMaker *maker = new CFGMaker(i);
		CFG_INDEX(i) = makers.count();
		makers.add(pair(i, maker));
		return *maker;
	}
}

/**
 * Process a new CFG starting at the given instruction.
 * @param i	Instruction starting the CFG.
 */
void AbstractCFGBuilder::processCFG(Inst *i) {
	FragTable<Inst *> entries;
	CFGMaker& m = maker(i);

	// traverse the BBs and mark them (ignore calls)
	scanCFG(i, entries);

	// build the basic blocks
	buildBBs(m, entries);

	// build the edges + call nodes
	buildEdges(m);

	// cleanup markers
	cleanBBs(entries);
}


/**
 */
AbstractCFGBuilder::AbstractCFGBuilder(Monitor& mon):
	Monitor(mon)
{}


AbstractCFGBuilder::~AbstractCFGBuilder() {
	for(int i = 0; i < makers.count(); i++) {
		CFG_INDEX(makers[i].fst).remove();
		delete makers[i].snd;
	}
	makers.clear();	
}


/**
 * Compute the CFG from the recorded makers;
 * @param ws	Current workspace.
 */
void AbstractCFGBuilder::process(WorkSpace *ws) {

	// record BB bounds
	for(int i = 0; i < bounds.count(); i++) {
		Inst *inst = ws->findInstAt(bounds[i]);
		if(!inst)
			log << "no instruction at " << bounds[i];
		else {
			BB(inst) = 0;
			if(logFor(LOG_BB))
				log << "\tset BB bound at " << bounds[i] << io::endl;
		}
	}

	// build the CFG
	for(int i = 0; i < makers.count(); i++)
		processCFG(makers[i].fst);
	
	// cleanup added bounds
	for(int i = 0; i < bounds.count(); i++) {
		Inst *inst = ws->findInstAt(bounds[i]);
		if(inst)
			inst->removeProp(BB);
	}
}


/**
 */
void AbstractCFGBuilder::configure(const PropList& props) {
	bounds = BB_BOUNDS(props);
}


/**
 * Configuration identifier, provides a list of BB start point
 * (whatever the control flow of the executable).
 * @ingroup cfg
 */
p::id<Bag<Address> > BB_BOUNDS("otawa::BB_BOUNDS");

} // otawa
