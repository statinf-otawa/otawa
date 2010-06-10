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

typedef enum action_t {
	DO_NOTHING = 0,
	DO_SWALLOW,
	DO_INSERT
} action_t;
static Identifier<action_t> ACTION("", DO_NOTHING);

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
 */
SilentFeature DELAYED_CFG_FEATURE("otawa::DELAYED_CFG_FEATURE", maker);


/**
 * This property is true on instructions part of a branch delay.
 *
 * @par Features
 * @li @ref otawa::DELAYED_CFG_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::Inst
 *
 */
Identifier<bool> DELAYED_INST("otawa::DELAYED_INST", false);


/**
 * This property is true on NOP instructions inserted due to branch delay.
 *
 * @par Features
 * @li @ref otawa::DELAYED_CFG_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::Inst
 *
 */
Identifier<bool> DELAYED_NOP("otawa::DELAYED_NOP", false);


/**
 * Build a single BB containing the instruction following the given BB.
 * @param cfg	Result CFG.
 * @param bb	Original BB.
 * @return		Built basic block.
 */
static BasicBlock *makeNext(VirtualCFG *cfg, BasicBlock *bb) {
	Inst *next = bb->lastInst()->nextInst();
	CodeBasicBlock *rbb = new CodeBasicBlock(next);
	rbb->setSize(next->size());
	cfg->addBB(rbb);
	return rbb;
}


static BasicBlock *makeNOp(WorkSpace *ws, VirtualCFG *cfg, BasicBlock *bb) {
	Inst *nop = ws->process()->newNOp(bb->lastInst()->nextInst()->address());
	CodeBasicBlock *rbb = new CodeBasicBlock(nop);
	rbb->setSize(nop->size());
	cfg->addBB(rbb);
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
DelayedBuilder::DelayedBuilder(void): CFGProcessor(reg) {
}


/**
 */
void DelayedBuilder::setup(WorkSpace *ws) {
	coll = new CFGCollection();
	cleaner = new Cleaner(ws->process());
}


/**
 */
void DelayedBuilder::cleanup(WorkSpace *ws) {
	track(COLLECTED_CFG_FEATURE, INVOLVED_CFGS(ws) = coll);
	addCleaner(COLLECTED_CFG_FEATURE, cleaner);
	ENTRY_CFG(ws) = coll->get(0);		// should cleaned at some time
	coll = 0;
}


/**
 */
void DelayedBuilder::processWorkSpace(WorkSpace *ws) {

	// mark instructions
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	for(CFGCollection::Iterator cfg(coll); cfg; cfg++) {

		// create the virtual CFG
		VirtualCFG *vcfg = new VirtualCFG(false);
		this->coll->add(vcfg);
		cfg_map.put(cfg, vcfg);

		// scan the blocks for delayed branches
		for(CFG::BBIterator bb(cfg); bb; bb++) {
			Inst *last = bb->lastInst();
			if(last) {
				switch(DELAYED(last)) {
				case DELAYED_None:
					break;
				case DELAYED_Always:
					DELAYED_INST(last->nextInst()) = true;
					ACTION(bb) = DO_SWALLOW;
					break;
				case DELAYED_Taken:
					DELAYED_INST(last->nextInst()) = true;
					ACTION(bb) = DO_INSERT;
					break;
				}
			}
		}
	}

	// usual CFG processing
	CFGProcessor::processWorkSpace(ws);
}


/**
 */
void DelayedBuilder::fix(Edge *oedge, Edge *nedge, map_t& map) {
	if(oedge->kind() != Edge::VIRTUAL_CALL)
		return;

	// virtual return
	BasicBlock *oreturn = VIRTUAL_RETURN_BLOCK(oedge->source());
	if(oreturn) {
		BasicBlock *nreturn = map.get(oreturn);
		ASSERT(nreturn);
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
 */
void DelayedBuilder::processCFG(WorkSpace *ws, CFG *cfg) {
	genstruct::HashTable<BasicBlock *, BasicBlock *> map;
	VirtualCFG *vcfg = cfg_map.get(cfg, 0);
	ASSERT(vcfg);

	// add entry
	vcfg->addBB(vcfg->entry());

	// build the basic blocks
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		BasicBlock *vbb = 0;
		if(bb->isEnd()) {
			if(bb->isEntry())
				vbb = vcfg->entry();
			else {
				ASSERT(bb->isExit());
				vbb = vcfg->exit();
			}
		}
		else {
			Inst *first = bb->firstInst();
			size_t size = bb->size();
			ASSERT(first);
			if(DELAYED_INST(first)) {
				size -= first->size();
				first = first->nextInst();
			}
			if(ACTION(bb) == DO_SWALLOW)
				size += bb->lastInst()->nextInst()->size();
			CodeBasicBlock *cbb = new CodeBasicBlock(first);
			cbb->setSize(size);
			vcfg->addBB(cbb);
			vbb = cbb;
		}
		map.put(bb, vbb);
	}

	// build the edges
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		BasicBlock *vbb = map.get(bb, 0);
		ASSERT(vbb);

		// no insertion
		if(ACTION(bb) != DO_INSERT) {
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				if(edge->kind() != Edge::CALL)
					fix(edge, new Edge(vbb, map.get(edge->target(), 0), edge->kind()), map);
				else if(!edge->calledCFG())
					new Edge(vbb, 0, Edge::CALL);
				else
					new Edge(vbb, cfg_map.get(edge->calledCFG(), 0)->entry(), Edge::CALL);
			}
		}

		// insertion
		else
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				BasicBlock *ibb;
				switch(edge->kind()) {
				case Edge::CALL:
					ibb = makeNext(vcfg, bb);
					new Edge(vbb, ibb, Edge::TAKEN);
					if(!edge->calledCFG())
						new Edge(ibb, 0, Edge::CALL);
					else
						new Edge(ibb, cfg_map.get(edge->calledCFG(), 0)->entry(), Edge::CALL);
					break;
				case Edge::NOT_TAKEN:
					ibb = makeNOp(ws,vcfg, bb);
					goto link;
				default:
					ibb = makeNext(vcfg, bb);
				link:
					fix(edge, new Edge(vbb, ibb, edge->kind()), map);
					new Edge(ibb, map.get(edge->target(), 0), Edge::NOT_TAKEN);
					break;
				}
			}
	}

	// finalization
	vcfg->addBB(vcfg->exit());
	vcfg->numberBBs();
}


/**
 */
DelayedBuilder::Cleaner::~Cleaner(void) {
	for(genstruct::SLList<Inst *>::Iterator inst(nops); inst; inst++)
		proc->deleteNop(inst);
}

} // otawa
