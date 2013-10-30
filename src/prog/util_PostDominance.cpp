/*
 *	$Id$
 *	PostDominance class implementation
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

#include <otawa/util/PostDominance.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/cfg.h>
#include <otawa/prop/DeletableProperty.h>

using namespace otawa::dfa;

namespace otawa {


/**
 * Identifier of annotation containing reverse-postdominance information.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<BitSet *> REVERSE_POSTDOM("otawa::REVERSE_POSTDOM", 0);


/**
 * This is the Problem used to instanciate DFAEngine for computing the
 * reverse postdomination relation. For each basic block, the set of postdominators
 * is computed and hooked to the basic block. Then, a simple bit test is
 * used for testing the relation.
 */

class PostDominanceProblem {
	CFG *cfg;
	int size;
public:
	PostDominanceProblem(CFG *_cfg) {
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
		if(bb->isExit())
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
	void free(BitSet *set) { delete set; }
};



/**
 * @class PostDominance
 * This CFG processor computes and hook to the CFG the postdominance relation
 * that, tehn, may be tested with @ref PostDominance::postDominate() function.
 */


/**
 * Test if the first basic block postdominates the second one.
 * @param bb1	Dominator BB.
 * @param bb2	Dominated BB.
 * @return		True if bb1 postdominates bb2.
 */
bool PostDominance::postDominates(BasicBlock *bb1, BasicBlock *bb2) {
	ASSERTP(bb1, "null BB 1");
	ASSERTP(bb2, "null BB 2");
	ASSERTP(bb1->cfg() == bb2->cfg(), "both BB are not owned by the same CFG");
	int index = bb1->number();
	ASSERTP(index >= 0, "no index for BB 1");
	BitSet *set = REVERSE_POSTDOM(bb2);
	ASSERTP(set, "no index for BB 2");

	ASSERTP(bb1 == bb2
		||	!REVERSE_POSTDOM(bb1)->contains(bb2->number())
		||  !REVERSE_POSTDOM(bb2)->contains(bb1->number()),
			"CFG with disconnected nodes");

	return set->contains(index);
}


/**
 * @fn bool PostDominance::isDominated(BasicBlock *bb1, BasicBlock *bb2);
 * Test if the first block is postdominated by the second one.
 * @param bb1	Dominated BB.
 * @param bb2	Dominator BB.
 * @return		True if bb2 postdominates bb1.
 */


/**
 * Computes the postdomination relation.
 */
void PostDominance::processCFG(WorkSpace *fw, CFG *cfg) {
	ASSERT(cfg);
	PostDominanceProblem dp(cfg);
	dfa::IterativeDFA<PostDominanceProblem, BitSet, Successor> engine(dp, *cfg);
	engine.compute();
	for (CFG::BBIterator blocks(cfg); blocks; blocks++) {
	  BitSet *b = engine.outSet(blocks.item());
	  b = new BitSet(*b);
	  //blocks->addDeletable<BitSet *>(REVERSE_POSTDOM, b);
	  blocks->addProp(new DeletableProperty<BitSet *>(REVERSE_POSTDOM, b));
	}
}


/**
 * Check if the postdominance informance is available. Else compute it.
 * @param cfg	CFG to look at.
 */
void PostDominance::ensure(CFG *cfg) {
	if(!REVERSE_POSTDOM(cfg->entry())) {
		PostDominance postdom;
		postdom.processCFG(0, cfg);
	}
}


/**
 * The postdominance processors computes postdominance relation
 * on the current CFG.
 *
 * @Provided Features
 * @li @ref POSTDOMINANCE_FEATURE
 */
PostDominance::PostDominance(void): CFGProcessor("otawa::PostDominance", Version(1, 1, 0)) {
	provide(POSTDOMINANCE_FEATURE);
}


/**
 * This feature ensures that information about postdomination between nodes
 * of a CFG is vailable.
 *
 * @par Properties
 * @li @ref REVERSE_DOM (BasicBlock)
 */
Feature<PostDominance> POSTDOMINANCE_FEATURE("otawa::POSTDOMINANCE_FEATURE");


} // otawa
