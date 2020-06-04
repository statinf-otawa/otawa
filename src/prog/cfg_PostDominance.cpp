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

#include <otawa/cfg.h>
#include <otawa/cfg/PostDominance.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/prop/DeletableProperty.h>

using namespace otawa::dfa;

namespace otawa {


/**
 * Identifier of annotation containing reverse-post-dominance information.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<BitSet *> REVERSE_POSTDOM("otawa::REVERSE_POSTDOM", 0);


// default post-dominance
class DefaultPostDomInfo: public PostDomInfo {
public:
	virtual bool pdom(Block *b1, Block *b2)  {
		ASSERT(b1);
		ASSERT(b2);
		ASSERTP(b1->cfg() == b2->cfg(), "both BB are not owned by the same CFG");
		BitSet *set = REVERSE_POSTDOM(b2);
		ASSERT(set);
		return set->contains(b1->index());

	}
};


/**
 * This is the Problem used to instantiate DFAEngine for computing the
 * reverse post-domination relation. For each basic block, the set of post-dominators
 * is computed and hooked to the basic block. Then, a simple bit test is
 * used for testing the relation.
 */
class PostDominanceProblem {
	CFG *cfg;
	int size;
public:
	PostDominanceProblem(CFG *_cfg) {
		cfg = _cfg;
		size = _cfg->count();
	}

	BitSet *empty(void) {
		BitSet *result = new BitSet(size);
		result->fill();
		return result;
	}

	BitSet *gen(Block *bb) {
		BitSet *result = new BitSet(size);
		result->add(bb->index());
		return result;
	}

	BitSet *kill(Block *bb) {
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
 * @ingroup CFG
 * This CFG processor implements @ref POSTDOMINANCE_FEATURE.
 *
 * @par Used Features
 *
 * @par Provided Features
 * @li @ref POSTDOMINANCE_FEATURE
 */

/**
 * Computes the postdomination relation.
 */
void PostDominance::processCFG(WorkSpace *fw, CFG *cfg) {
	ASSERT(cfg);
	PostDominanceProblem dp(cfg);
	dfa::IterativeDFA<PostDominanceProblem, BitSet, CFG, Successor> engine(dp, cfg, cfg->exit());
	engine.compute();
	for (CFG::BlockIter blocks = cfg->blocks(); blocks(); blocks++) {
	  BitSet *b = engine.outSet(blocks.item());
	  b = new BitSet(*b);
	  //blocks->addDeletable<BitSet *>(REVERSE_POSTDOM, b);
	  blocks->addProp(new DeletableProperty<BitSet *>(REVERSE_POSTDOM, b));
	}
}

///
bool PostDominance::pdom(Block *b1, Block *b2) {
	ASSERT(b1);
	ASSERT(b2);
	const BitSet *set = REVERSE_POSTDOM(b2);
	ASSERT(set);
	return set->contains(b1->index());
}

///
void *PostDominance::interfaceFor(const AbstractFeature& feature) {
	if(&feature == &POSTDOMINANCE_FEATURE)
		return static_cast<PostDomInfo *>(this);
	else
		return nullptr;
}

///
void PostDominance::destroyCFG(WorkSpace *ws, CFG *g) {
	g->clean(REVERSE_POSTDOM);
}

/**
 */
p::declare PostDominance::reg = p::init("otawa::PostDominance", Version(2, 0, 0))
	.provide(POSTDOMINANCE_FEATURE)
	.base(CFGProcessor::reg)
	.maker<PostDominance>();

/**
 * The postdominance processors computes postdominance relation
 * on the current CFG.
 * @ingroup cfg
 *
 * @par Provided Features
 * @li @ref PDOM_INFO
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE
 */
PostDominance::PostDominance(p::declare& r): CFGProcessor(reg) {
}

/**
 * This feature ensures that information about postdomination between nodes
 * of a CFG is vailable.
 * @ingroup cfg
 *
 * @par Properties
 * @li @ref REVERSE_DOM (BasicBlock)
 */
p::interfaced_feature<PostDomInfo> POSTDOMINANCE_FEATURE("otawa::POSTDOMINANCE_FEATURE", new Maker<PostDominance>());

/**
 * Provide post-domination information.
 *
 * @par Hooks
 * @li @ref WorkSpace
 *
 * @par Providing Features
 * @li @ref POSTDOMINANCE_FEATURE
 */
Identifier<PostDomInfo *> PDOM_INFO("otawa::PDOM_INFO", 0);

/**
 * @class PostDomInfo
 * Allows to test if one block post-dominates another one.
 * @ingroup cfg
 */

/**
 */
PostDomInfo::~PostDomInfo(void) {
}

/**
 * @fn bool PostDomInfo::pdom(Block *b1, Block *b2);
 * Test if b1 post-dominates b2.
 * @param b1	First block.
 * @param b2	Second block.
 * @return		True if b1 post-dominates b2, false else.
 */

} // otawa
