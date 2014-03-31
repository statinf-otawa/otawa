/*
 *	$Id$
 *	DelayedBuilder
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
	genstruct::FragTable<inhstruct::DLList> lists;
	genstruct::FragTable<NOPInst> nops;
};


typedef enum action_t {
	DO_NOTHING = 0,
	DO_SWALLOW,
	DO_INSERT
} action_t;
static Identifier<action_t> ACTION("", DO_NOTHING);
static Identifier<BasicBlock *> DELAYED_TARGET("", 0);

static SilentFeature::Maker<DelayedBuilder> maker;
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
SilentFeature DELAYED_CFG_FEATURE("otawa::DELAYED_CFG_FEATURE", maker);


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
 * @param inst	Starting instruction.
 * @param n		Number of instruction to delay.
 * @return		Built basic block.
 */
BasicBlock *DelayedBuilder::makeBB(Inst *inst, int n) {
	CodeBasicBlock *rbb = new CodeBasicBlock(inst);
	rbb->setSize(size(inst, n));
	vcfg->addBB(rbb);
	return rbb;
}


/**
 * Build a block made of NOPs matching the given instructions.
 * @param inst	Instruction to start from.
 * @param n		Number of NOPs.
 * @return		Built basic block.
 */
BasicBlock *DelayedBuilder::makeNOp(Inst *inst, int n) {

	// build the list of instructions
	Inst *first = 0;
	ot::size size = 0;
	inhstruct::DLList *list = cleaner->allocList();
	for(; n; n--) {

		// make the new nop
		NOPInst *nop = cleaner->allocNop(inst->address(), inst->size());
		nop->append(*list);

		// go to next instruction
		if(!inst->nextInst())
			inst = workspace()->process()->findInstAt(inst->address());
		else
			inst = inst->nextInst();
	}

	// build the block itself
	//Inst *nop = workspace()->process()->newNOp(bb->lastInst()->nextInst()->address());
	CodeBasicBlock *rbb = new CodeBasicBlock(first);
	rbb->setSize(size);
	vcfg->addBB(rbb);
	return rbb;
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

Registration<DelayedBuilder> DelayedBuilder::reg(
	"otawa::DelayedBuilder",
	Version(1, 0, 0),
	p::base, &CFGProcessor::reg,
	p::require, &COLLECTED_CFG_FEATURE,
	p::invalidate, &COLLECTED_CFG_FEATURE,
	p::provide, &COLLECTED_CFG_FEATURE,
	p::provide, &DELAYED_CFG_FEATURE,
	p::end
);


/**
 */
DelayedBuilder::DelayedBuilder(void): CFGProcessor(reg), coll(0), cleaner(0), info(0), vcfg(0) {
}


/**
 */
void DelayedBuilder::setup(WorkSpace *ws) {

	// initialization
	coll = new CFGCollection();
	cleaner = new DelayedCleaner();

	// look for DELAYED2 support
	if(workspace()->isProvided(DELAYED2_FEATURE)) {
		info = DELAYED_INFO(workspace()->process());
		ASSERT(info);
	}
	else
		info = 0;
}


/**
 */
void DelayedBuilder::cleanup(WorkSpace *ws) {

	// track feature cleanup
	track(COLLECTED_CFG_FEATURE, INVOLVED_CFGS(ws) = coll);
	addCleaner(COLLECTED_CFG_FEATURE, cleaner);
	ENTRY_CFG(ws) = coll->get(0);		// should be cleaned at some time

	// cleanup
	coll = 0;
	cleaner = 0;
	cfg_map.clear();
	map.clear();
	vcfg = 0;
}


/**
 * Mark the instructions with actions.
 * @param cfg	CFG to mark.
 */
void DelayedBuilder::mark(CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		Inst *control = bb->controlInst();
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
 */
void DelayedBuilder::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);

	// phase 1: mark and create CFG and BB
	for(CFGCollection::Iterator cfg(coll); cfg; cfg++) {

		// mark the instructions
		mark(cfg);

		// build CFG
		vcfg = new VirtualCFG(false);
		this->coll->add(vcfg);
		cfg_map.put(cfg, vcfg);
	}

	// phase 2: build the BB
	for(CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		vcfg = cfg_map.get(cfg, 0);
		ASSERT(vcfg);
		vcfg->addBB(vcfg->entry());
		buildBB(cfg);
	}

	// phase 2: create edges
	CFGProcessor::processWorkSpace(ws);
}


/**
 */
void DelayedBuilder::fix(Edge *oedge, Edge *nedge) {
	if(oedge->kind() != Edge::VIRTUAL_CALL)
		return;

	// virtual return
	BasicBlock *oreturn = VIRTUAL_RETURN_BLOCK(oedge->source());
	if(oreturn) {
		BasicBlock *nreturn = map.get(oreturn, 0);

		// returning to single-instruction delayed BB -> no match in map
		if(!nreturn) {
			BasicBlock::OutIterator edge(oreturn);
			if(!edge)
				throw otawa::Exception(_ << "delayed mono-instruction BB without successor at " << oreturn->address() << ": " << oreturn);
			if(edge->kind() == Edge::NOT_TAKEN) {
				nreturn = map.get(edge->target());
				edge++;
			}
			if(edge)
				throw otawa::Exception(_ << "branch in delayed instruction unsupported at " << oreturn->address() << ": " << oreturn->firstInst());
		}

		// reset virtual return block
		VIRTUAL_RETURN_BLOCK(nedge->source()) = nreturn;
	}

	// called CFG
	CFG *cfg = CALLED_CFG(oedge);
	if(cfg)
		CALLED_CFG(nedge) = cfg;

	// recursive
	if(RECURSIVE_LOOP(oedge))
		RECURSIVE_LOOP(nedge) = true;
}


/**
 * Clone an existing edge.
 * @param edge		Cloned edge.
 * @param source	Source BB.
 * @param kind		Kind of the created edge.
 */
void DelayedBuilder::cloneEdge(Edge *edge, BasicBlock *source, Edge::kind_t kind) {
	if(logFor(LOG_INST))
		cerr << "\t\t\t" << edge << io::endl;
	if(edge->kind() == Edge::CALL) {
		BasicBlock *target;
		if(edge->target() == 0)
			target = 0;
		else
			target = cfg_map.get(edge->calledCFG(), 0)->entry();
		Edge *vedge = makeEdge(source, target, Edge::CALL);
		if(target)
			CALLED_BY(target->cfg()).add(vedge);
	}
	else {
		// target BB of a delayed BB with multiple entries
		BasicBlock *target = DELAYED_TARGET(edge->target());
		if(!target)
			target = map.get(edge->target(), 0);
		ASSERT(target);
		fix(edge, makeEdge(source, target, kind));
	}
}


/**
 * Insert a basic block into an edge.
 * @param edge	Split edge.
 * @param ibb	Inserted basic block.
 * @param map	Used map.
 */
void DelayedBuilder::insert(Edge *edge, BasicBlock *ibb) {
	BasicBlock *source = map.get(edge->source(), 0);
	ASSERT(source);

	// call case
	if(edge->kind() == Edge::CALL) {
		BasicBlock *target;
		if(edge->target() == 0)
			target = 0;
		else
			target = cfg_map.get(edge->calledCFG(), 0)->entry();
		makeEdge(source, ibb, Edge::TAKEN);
		Edge *vedge = makeEdge(ibb, target, Edge::CALL);
		if(target)
			CALLED_BY(target->cfg()).add(vedge);
	}

	// other cases
	else {
		// target BB of a delayed BB with multiple entries
		BasicBlock *target = DELAYED_TARGET(edge->target());

		// else find normal target
		if(!target)
			target = map.get(edge->target(), 0);

		// make edges
		fix(edge, makeEdge(source, ibb, edge->kind()));
		makeEdge(ibb, target, Edge::NOT_TAKEN);
	}
}


/**
 * Build the BB.
 * @param cfg	Current CFG.
 */
void DelayedBuilder::buildBB(CFG *cfg) {

	// build the basic blocks
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		BasicBlock *vbb = 0, *delayed = 0;

		// case of ends
		if(bb->isEnd()) {
			if(bb->isEntry())
				vbb = vcfg->entry();
			else {
				ASSERT(bb->isExit());
				vbb = vcfg->exit();
			}
		}

		// other basic blocks
		else {
			Inst *first = bb->firstInst();
			ot::size size = bb->size();
			ASSERT(first);

			// start of BB is delayed instructions?
			int dcnt = TO_DELAY(first);
			if(dcnt) {
				if(logFor(LOG_BB))
					log << "\t\t" << *bb << " reduced due to " << dcnt << " delayed instruction\n";

				// remove delayed mono-instruction BB
				if(bb->countInstructions() <= dcnt) {
					if(logFor(LOG_BB))
						log << "\t\too small delayed BB removed: " << *bb << io::endl;
					continue;
				}

				// add undirect BB for other entering edges
				for(BasicBlock::InIterator edge(bb); edge; edge++)
					if(edge->kind() != Edge::NOT_TAKEN
					&& edge->kind() != Edge::VIRTUAL_RETURN) {
						delayed = makeBB(first, dcnt);
						DELAYED_TARGET(bb) = delayed;
						break;
					}

				// reduce delayed BB
				size -= this->size(first, dcnt);
				first = next(first, dcnt);
			}

			// perform swallowing
			if(ACTION(bb) == DO_SWALLOW) {
				int ecnt = count(bb->controlInst());
				ot::size esize = this->size(bb->controlInst(), ecnt);
				if(logFor(LOG_BB))
					log << "\t\t" << *bb << " extended by " << ecnt << " delayed instruction(s) (" << esize << " bytes)\n";
				size += esize;
			}

			// create block
			CodeBasicBlock *cbb = new CodeBasicBlock(first);
			cbb->setSize(size);
			vcfg->addBB(cbb);
			vbb = cbb;

			// delayed edge
			if(delayed) {
				ASSERT(delayed->cfg() == vbb->cfg() && delayed->cfg() == vcfg);
				makeEdge(delayed, vbb, Edge::NOT_TAKEN);
			}
		}

		// record the new BB
		map.put(bb, vbb);
	}
}


/**
 * Build the edges.
 * @param cfg	Current CFG.
 */
void DelayedBuilder::buildEdges(CFG *cfg) {
	if(logFor(LOG_CFG))
		cerr << "\tbuild edges for " << cfg << io::endl;
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		if(logFor(LOG_BB))
			cerr << "\t\t" << *bb << io::endl;
		BasicBlock *vbb = map.get(bb, 0);
		if(!vbb)
			continue;

		switch(ACTION(bb)) {

		// no delay
		case DO_NOTHING:
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				cloneEdge(edge, vbb, edge->kind());
			break;

		// just swallowing
		case DO_SWALLOW:
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				if(edge->kind() == Edge::CALL) {
					CFG *ccfg = cfg_map.get(edge->calledCFG(), 0);
					ASSERT(ccfg);
					new Edge(vbb, ccfg->firstBB(), Edge::CALL);
					ENTRY(ccfg->firstBB()) = ccfg;
				}
				else {
					if(map.get(edge->target(), 0))
						cloneEdge(edge, vbb, edge->kind());
					else
						// relink successors of removed mono-instruction BB
						for(BasicBlock::OutIterator out(edge->target()); out; out++)
							cloneEdge(out, vbb, edge->kind());
				}
			}
			break;

		// do insertion
		case DO_INSERT:
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {

				// not taken
				if(edge->kind() == Edge::NOT_TAKEN || edge->kind() == Edge::VIRTUAL_RETURN) {
					BasicBlock *nop = makeNOp(bb->first);
					makeEdge(vbb, nop, edge->kind());
					BasicBlock *vtarget = map.get(edge->target(), 0);

					// simple not-taken edge
					if(vtarget)
						makeEdge(nop, vtarget, Edge::NOT_TAKEN);

					// relink successors of removed mono-instruction BB
					else
						for(BasicBlock::OutIterator out(edge->target()); out; out++)
							cloneEdge(out, nop, out->kind());
				}

				// other edges
				else {
					BasicBlock *ibb = makeBB(bb->lastInst()->nextInst());
					insert(edge, ibb);
				}
			}
			break;
		}
	}
}



/**
 */
void DelayedBuilder::processCFG(WorkSpace *ws, CFG *cfg) {
	vcfg = cfg_map.get(cfg, 0);
	ASSERT(vcfg);
	buildEdges(cfg);
	vcfg->addBB(vcfg->exit());
	vcfg->numberBBs();
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
 * Define the count of instructions before the given delayed branch be effective.
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
Edge *DelayedBuilder::makeEdge(BasicBlock *src, BasicBlock *tgt, Edge::kind_t kind) {
	ASSERT(src->cfg() == vcfg && tgt->cfg() == vcfg);
	return new Edge(src, tgt, kind);
}

} // otawa
