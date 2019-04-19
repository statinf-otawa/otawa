/*
 *	DelayedBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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

#include <otawa/cfg/DelayedBuilder.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>

namespace otawa {

Identifier<int> TO_DELAY("", 0);

// Tools
class NOPInst: public Inst {
public:
	inline NOPInst(void): _size(0) { }
	inline NOPInst(Address addr, ot::size size): _addr(addr), _size(size) { }
	void append(inhstruct::DLList& list) {list.addLast(this); }
	virtual void dump(io::Output &out) { out << "<delayed-nop>"; }
	virtual kind_t kind(void) { return IS_ALU | IS_INT; }
	virtual Address address (void) const { return _addr; }
	virtual t::uint32 size(void) const { return _size; }
private:
	Address _addr;
	ot::size _size;
};


class DelayedCleaner: public elm::Cleaner {
public:

	inhstruct::DLList *allocList(void) {
		lists.add(inhstruct::DLList());
		return &lists[lists.count() - 1];
	}

	NOPInst *allocNop(Address addr, ot::size size) {
		nops.add(NOPInst(addr, size));
		NOPInst *nop = &nops[nops.count() - 1];
		DELAYED_NOP(nop) = true;
		return nop;
	}

private:
	FragTable<inhstruct::DLList> lists;
	FragTable<NOPInst> nops;
};


typedef enum action_t {
	DO_NOTHING = 0,
	DO_SWALLOW,
	DO_INSERT
} action_t;
static Identifier<action_t> ACTION("", DO_NOTHING);
static Identifier<Block *> DELAYED_TARGET("", 0);

/**
 * This feature informs that the current microprocessor supports
 * delayed branches and that the CFG has been changed to reflect effect
 * of delay branches.
 *
 * @par Default Processor
 * @li @ref otawa::DelayedBuilder
 *
 * @par Properties
 * @li @ref otawa::DELAYED_INST
 * @li @ref otawa::DELAYED_NOP
 *
 * @ingroup cfg
 */
p::feature DELAYED_CFG_FEATURE("otawa::DELAYED_CFG_FEATURE", new Maker<DelayedBuilder>());


/**
 * This property is set on instruction of a delayed branch.
 *
 * @par Features
 * @li @ref otawa::DELAYED_CFG_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::Inst
 *
 * @ingroup cfg
 */
Identifier<bool> DELAYED_INST("otawa::DELAYED_INST", false);


/**
 * This property is true on NOP instructions insserted due to branch delay.
 *
 * @par Features
 * @li @ref otawa::DELAYED_CFG_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::Inst
 *
 * @ingroup cfg
 */
Identifier<bool> DELAYED_NOP("otawa::DELAYED_NOP", false);


/**
 * Build a single BB containing the instruction following the given instruction.
 * @param maker	CFG maker to use.
 * @param inst	Starting instruction.
 * @param n		Number of instruction to delay.
 * @return		Built basic block.
 */
BasicBlock *DelayedBuilder::makeBB(Inst *inst, int n) {
	return build(inst, n);
}


/**
 * Build a block made of NOPs matching the given instructions.
 * @param inst	Instruction to start from.
 * @param n		Number of NOPs.
 * @return		Built basic block.
 */
BasicBlock *DelayedBuilder::makeNOp(Inst *inst, int n) {

	// build the list of instructions
	Vector<Inst *> insts;
	for(; n; n--) {

		// make the new nop
		NOPInst *nop = cleaner->allocNop(inst->address(), inst->size());
		insts.add(nop);

		// go to next instruction
		if(!inst->nextInst())
			inst = workspace()->process()->findInstAt(inst->address());
		else
			inst = inst->nextInst();
	}

	// build the block itself
	return build(insts.detach());
}


/**
 * @class DelayedBuilder
 * This processor handle the delayed branch information provided by
 * @ref otawa::DELAYED_FEATURE to transform the CFG to let other processor
 * ignore the delayed branch effects.
 *
 * If a branch is denoted as @ref otawa::DELAYED_ALWAYS, the first instruction
 * of the basic block in sequence is moved in the branch basic block (just after
 * the branch).
 *
 * If a branch is denoted as @ref otawa::DELAYED_TAKEN, a basic block is inserted
 * on the taken edge containing only the first instruction of the basic block
 * in sequence and a basic block containing a NOP instruction is inserted in
 * the edge of sequential transfer of control (to take into account the delayed
 * instruction effect on memory hierarchy).
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE
 * @li @ref DELAYED_FEATURE
 *
 * @par Provided Features
 * @li @ref COLLECTED_CFG_FEATURE
 * @li @ref DELAYED_CFG_FEATURE
 *
 * @ingroup cfg
 */

p::declare DelayedBuilder::reg = p::init("otawa::DelayedBuilder", Version(2, 0, 0))
	.base(CFGTransformer::reg)
	.provide(DELAYED_CFG_FEATURE)
	.maker<DelayedBuilder>();


/**
 */
DelayedBuilder::DelayedBuilder(void): CFGTransformer(reg), cleaner(0), info(0) {
}


/**
 */
void DelayedBuilder::setup(WorkSpace *ws) {
	CFGTransformer::setup(ws);
	if(workspace()->isProvided(DELAYED2_FEATURE)) {
		info = DELAYED_INFO(workspace()->process());
		ASSERT(info);
	}
	else
		info = 0;
	cleaner = new DelayedCleaner();
}


/**
 */
void DelayedBuilder::cleanup(WorkSpace *ws) {
	CFGTransformer::cleanup(ws);
	addCleaner(COLLECTED_CFG_FEATURE, cleaner);
}


/**
 * Mark the instructions with actions.
 * @param cfg	CFG to mark.
 */
void DelayedBuilder::mark(CFG *cfg) {
	for(CFG::BlockIter b = cfg->blocks(); b(); b++)
		if(b->isBasic()) {
			BasicBlock *bb = b->toBasic();
			Inst *control = bb->control();
			if(control) {
				switch(type(control)) {
				case DELAYED_None:
					break;
				case DELAYED_Always:
					TO_DELAY(next(control)) = count(control);
					ACTION(bb) = DO_SWALLOW;
					break;
				case DELAYED_Taken:
					TO_DELAY(next(control)) = count(control);
					ACTION(bb) = DO_INSERT;
					break;
				}
			}
	}
}


/**
 * Clone an existing edge.
 * @param edge		Cloned edge.
 * @param source	Source BB.
 * @param kind		Kind of the created edge.
 */
void DelayedBuilder::cloneEdge(Edge *edge, Block *source, t::uint32 kind) {
	if(logFor(LOG_INST))
		cerr << "\t\t\t" << edge << io::endl;
	Block *target = DELAYED_TARGET(edge->target());
	if(!target)
		target = get(edge->target());
	ASSERT(target);
	build(source, target, kind);
}


/**
 * Insert a basic block into an edge.
 * @param edge	Split edge.
 * @param ibb	Inserted basic block.
 * @param map	Used map.
 */
void DelayedBuilder::insert(Edge *edge, BasicBlock *ibb) {
	Block *source = get(edge->source());
	ASSERT(source);

	// target BB of a delayed BB with multiple entries
	Block *target = DELAYED_TARGET(edge->target());

	// else find normal target
	if(!target)
		target = get(edge->target());

	// make edges
	makeEdge(source, ibb, edge->flags());
	makeEdge(ibb, target, Edge::NOT_TAKEN);
}


/**
 */
void DelayedBuilder::transform(CFG *g, CFGMaker& m) {
	mark(g);
	buildBB(g, m);
	buildEdges(g, m);
}


/**
 * Build the BB.
 * @param cfg	Current CFG.
 * @param maker	Maker of the new CFG.
 */
void DelayedBuilder::buildBB(CFG *cfg, CFGMaker& maker) {
	if(logFor(LOG_BLOCK))
		log << "\t\tbuild states\n";

	// build the basic blocks
	for(CFG::BlockIter b = cfg->blocks(); b(); b++) {
		Block *delayed = 0;

		// all except BB
		if(!b->isBasic())
			CFGTransformer::transform(*b);

		// other basic blocks
		else {
			BasicBlock *bb = b->toBasic();
			Inst *first = bb->first();
			//ot::size size = bb->size();
			int cnt = bb->count();
			ASSERT(first);

			// start of BB is delayed instructions?
			int dcnt = TO_DELAY(first);
			if(dcnt) {
				if(logFor(LOG_BB))
					log << "\t\t" << bb << " reduced due to " << dcnt << " delayed instruction\n";

				// remove delayed mono-instruction BB
				if(bb->count() <= dcnt) {
					if(logFor(LOG_BB))
						log << "\t\too small delayed BB removed: " << *bb << io::endl;
					continue;
				}

				// add indirect BB for other entering edges
				// TODO delayed should be built once!
				for(Block::EdgeIter edge = bb->ins(); edge(); edge++)
					if(edge->isTaken()) {
						delayed = build(first, dcnt);
						DELAYED_TARGET(bb) = delayed;
						break;
					}

				// reduce delayed BB
				cnt -= dcnt;
				first = next(first, dcnt);
			}

			// perform swallowing
			if(ACTION(bb) == DO_SWALLOW) {
				int ecnt = count(bb->control());
				if(logFor(LOG_BB))
					log << "\t\t" << bb << " extended by " << ecnt << " delayed instruction(s)\n";
				cnt += ecnt;
			}

			// create block
			BasicBlock *cbb = build(first, cnt);
			map(bb, cbb);

			// delayed edge
			if(delayed)
				build(delayed, cbb, Edge::NOT_TAKEN);
		}
	}
}


/**
 * Build the edges.
 * @param cfg	Current CFG.
 * @param maker	Maker of the new CFG.
 */
void DelayedBuilder::buildEdges(CFG *cfg, CFGMaker& maker) {
	if(logFor(LOG_BLOCK))
		cerr << "\t\tbuild edges for " << cfg << io::endl;

	for(CFG::BlockIter b = cfg->blocks(); b(); b++) {
		if(logFor(LOG_BB))
			cerr << "\t\t" << *b << io::endl;
		if(!isMapped(*b))
			continue;
		Block *vb = get(*b);

		switch(ACTION(*b)) {

		// no delay
		case DO_NOTHING:
			for(Block::EdgeIter edge = b->outs(); edge(); edge++)
				cloneEdge(*edge, vb, edge->flags());
			break;

		// just swallowing
		case DO_SWALLOW:
			for(Block::EdgeIter edge = b->outs(); edge(); edge++) {
				if(isMapped(edge->target()))
					cloneEdge(*edge, vb, edge->flags());
				else
					// relink successors of removed mono-instruction BB
					// TODO fix it to support multiple swallowing (rare but may happen)
					for(Block::EdgeIter out = edge->target()->outs(); out(); out++)
						cloneEdge(*out, vb, edge->flags());
			}
			break;

		// do insertion
		case DO_INSERT:
			{
				BasicBlock *bb = b->toBasic();
				for(Block::EdgeIter edge = b->outs(); edge(); edge++) {

					// not taken
					//if(edge->isNotTaken() == Edge::NOT_TAKEN) {
					if(edge->isNotTaken()) {
						Block *nop = makeNOp(bb->first());
						makeEdge(vb, nop, edge->flags());
						Block *vtarget = get(edge->target());

						// simple not-taken edge
						if(vtarget)
							makeEdge(nop, vtarget, Edge::NOT_TAKEN);

						// relink successors of removed mono-instruction BB
						else
							for(Block::EdgeIter out = edge->target()->outs(); out(); out++)
								cloneEdge(*out, nop, out->flags());
					}

					// other edges
					else {
						BasicBlock *ibb = makeBB(bb->last()->nextInst());
						insert(*edge, ibb);
					}
				}
			}
			break;
		}
	}
}


/**
 * Define the type of delayed branch.
 * @param inst	Branch instruction.
 * @return		Type of delayed branch.
 */
delayed_t DelayedBuilder::type(Inst *inst) {
	if(!info)
		return DELAYED(inst);
	else
		return info->type(inst);
}


/**
 * Define the count of instructions before the given delayed branch being effective.
 * @param inst	Delayed branch.
 * @return		Count of instructions.
 */
int DelayedBuilder::count(Inst *inst) {
	if(!info)
		return 1;
	else
		return info->count(inst);
}


/**
 * Compute the size of the delayed instruction.
 * @param inst	First instruction.
 * @param n		Number of instructions in the delayed part.
 * @return		Size of the delayed instructions.
 */
ot::size DelayedBuilder::size(Inst *inst, int n) {
	ot::size size = 0;
	for(; n; n--) {
		size += inst->size();
		if(inst->nextInst() && inst->nextInst()->address() == inst->topAddress())
			inst = inst->nextInst();
		else {
			inst = workspace()->process()->findInstAt(inst->topAddress());
			if(!inst)
				throw ProcessorException(*this, _ << " cannot fetch instruction from " << inst->topAddress());
		}
	}
	return size;
}


/**
 * Get the instruction following the n next instruction.
 * @param inst	Instruction to start from.
 * @param n		Instruction to skip.
 * @return		Next instruction.
 */
Inst *DelayedBuilder::next(Inst *inst, int n) {
	for(; n; n--) {
		if(inst->nextInst() && inst->nextInst()->address() == inst->topAddress())
			inst = inst->nextInst();
		else {
			inst = workspace()->process()->findInstAt(inst->topAddress());
			if(!inst)
				throw ProcessorException(*this, _ << " cannot fetch instruction from " << inst->topAddress());
		}
	}
	return inst;
}


/**
 * Build a new virtual edge.
 * @param src	Source BB.
 * @param tgt	Target BB.
 * @param kind	Kind of the edge.
 */
Edge *DelayedBuilder::makeEdge(Block *src, Block *tgt, t::uint32 kind) {
	return build(src, tgt, kind);
}


/**
 */
void DelayedBuilder::processWorkSpace(WorkSpace *ws) {
	if(!ws->isProvided(otawa::DELAYED_FEATURE))
		warn("running this processor is not useful as the architecture does not support delayed branches");
	CFGTransformer::processWorkSpace(ws);
}

} // otawa
