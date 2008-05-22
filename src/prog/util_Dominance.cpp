/*
 *	$Id$
 *	Dominance class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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

#include <otawa/util/Dominance.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/cfg.h>

using namespace otawa::dfa;

namespace otawa {


/**
 * Identifier of annotation containing reverse-dominance information.
 * 
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<BitSet *> REVERSE_DOM("otawa::reverse_dom", 0);


/**
 * Identifier for marking basic blocks that are entries of loops.
 * 
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<bool> LOOP_HEADER("otawa::loop_header", false);


/**
 * This is the Problem used to instanciate DFAEngine for computing the 
 * reverse domination relation. For each basic block, the set of dominators 
 * is computed and hooked to the basic block. Then, a simple bit test is 
 * used for testing the relation.
 */

class DominanceProblem {
	CFG *cfg;
	int size;
public:
	DominanceProblem(CFG *_cfg) {
		cfg = _cfg;
		size = _cfg->countBB();
	}
	
	BitSet *empty(void) {
		BitSet *result = new BitSet(size);
		result->fill();
		return result;
	}
	
	BitSet *gen(BasicBlock *bb) {
		BitSet *result = new BitSet(size);
		result->add(bb->number());
		return result;
	}
	
	BitSet *kill(BasicBlock *bb) {
		BitSet *result = new BitSet(size);
		if(bb->isEntry())
			result->fill();
		return(result);
	}
	bool equals(BitSet *set1, BitSet *set2) {
		return(set1->equals(*set2));
	}
	void reset(BitSet *set) {
		set->fill();
	}
	void merge(BitSet *set1, BitSet *set2) {
		set1->mask(*set2);
	}
	void set(BitSet *dset, BitSet *tset) {
		*dset = *tset;
	}
	void add(BitSet *dset, BitSet *tset) {
		dset->add(*tset);
	}
	void diff(BitSet *dset, BitSet *tset) {
		dset->remove(*tset);
	}
};



/**
 * @class Dominance
 * This CFG processor computes and hook to the CFG the dominance relation
 * that, tehn, may be tested with @ref Dominance::dominate() function.
 */


/**
 * Test if the first basic block dominates the second one.
 * @param bb1	Dominator BB.
 * @param bb2	Dominated BB.
 * @return		True if bb1 dominates bb2.
 */
bool Dominance::dominates(BasicBlock *bb1, BasicBlock *bb2) {
	ASSERTP(bb1, "null BB 1");
	ASSERTP(bb2, "null BB 2");
	ASSERTP(bb1->cfg() == bb2->cfg(), "both BB are not owned by the same CFG");
	int index = bb1->number();
	ASSERTP(index >= 0, "no index for BB 1");
	BitSet *set = REVERSE_DOM(bb2);
	ASSERTP(set, "no index for BB 2");
	ASSERTP(bb1 == bb2	
		||	!REVERSE_DOM(bb1)->contains(bb2->number())
		||  !REVERSE_DOM(bb2)->contains(bb1->number()),
			"CFG with disconnected nodes (BB "
				<< bb1->number() << ":" << bb1->address()
				<< ", BB " << bb2->number() << ":" << bb2->address() << ")");
	return set->contains(index);
}


/**
 * @fn bool Dominance::isDominated(BasicBlock *bb1, BasicBlock *bb2);
 * Test if the first block is dominated by the second one.
 * @param bb1	Dominated BB.
 * @param bb2	Dominator BB.
 * @return		True if bb2 dominates bb1.
 */


/**
 * Computes the domination relation.
 */
void Dominance::processCFG(WorkSpace *fw, CFG *cfg) {
	assert(cfg);
	DominanceProblem dp(cfg);
	dfa::IterativeDFA<DominanceProblem, BitSet> engine(dp, *cfg);
	engine.compute();
	for (CFG::BBIterator blocks(cfg); blocks; blocks++) {
	  BitSet *b = engine.outSet(blocks.item());
	  b = new BitSet(*b);
	  blocks->addDeletable<BitSet *>(REVERSE_DOM, b);
	}
	markLoopHeaders(cfg);
}


/**
 * Using a CFG where dominance relation has been computed, mark loop headers.
 * @param cfg		CFG to process.
 * @param headers	Collection filled with found headers.
 */
void Dominance::markLoopHeaders(CFG *cfg,
elm::MutableCollection<BasicBlock *> *headers) {
	assert(cfg);
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			if(edge->target()
			&& edge->kind() != Edge::CALL
			&& dominates(edge->target(), bb)) {
				LOOP_HEADER(edge->target()) = true;
				if(headers)
					headers->add(bb);
				break;
			}
	}
}


/**
 * Test if the given baic block is a loop header.
 * @return True if it is a loop ehader, false else.
 */
bool Dominance::isLoopHeader(BasicBlock *bb) {
	for(BasicBlock::InIterator edge(bb); edge; edge++)
		if(edge->kind() != Edge::CALL
		&& Dominance::dominates(bb, edge->source()))
			return true;
	return false;
}


/**
 * Check if the dominance informance is available. Else compute it.
 * @param cfg	CFG to look at.
 */
void Dominance::ensure(CFG *cfg) {
	if(!REVERSE_DOM(cfg->entry())) {
		Dominance dom;
		dom.processCFG(0, cfg);
	}
}


/**
 * The dominance processors computes dominance relation and the loop headers
 * on the current CFG.
 * 
 * @Provided Features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 */
Dominance::Dominance(void): CFGProcessor("otawa::dominance", Version(1, 1, 0)) {
	provide(DOMINANCE_FEATURE);
	provide(LOOP_HEADERS_FEATURE);
}


/**
 * This feature ensures that information about domination between nodes
 * of a CFG is vailable.
 * 
 * @par Properties
 * @li @ref REVERSE_DOM (BasicBlock)
 */
Feature<Dominance> DOMINANCE_FEATURE("dominance");


/**
 * This feature ensures that all loop header are marked with a @ref LOOP_HEADER
 * property.
 * 
 * @Properties
 * @li @ref LOOP_HEADER (BasicBlock).
 */
Feature<Dominance> LOOP_HEADERS_FEATURE("loop_headers");

} // otawa
