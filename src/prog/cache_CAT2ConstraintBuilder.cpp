/*
 *	$Id$
 *	CAT2ConstraintBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
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

#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ipet/TrivialInstCacheManager.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/CAT2ConstraintBuilder.h>
#include <otawa/cache/cat2/LinkedBlocksDetector.h>
#include <otawa/stats/BBStatCollector.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {
	
using namespace cache;


class CacheCollector: public BBStatCollector {
public:
	CacheCollector(WorkSpace *ws, cstring name): BBStatCollector(ws), _name(name) {
		system = SYSTEM(ws);
		ASSERT(system);
	}

	virtual cstring name(void) const { return _name; }
	virtual cstring unit(void) const { return "cache access"; }
	virtual bool isEnum(void) const { return false; }
	virtual const cstring valueName(int value) { return ""; }

	virtual int total(BasicBlock *bb) {
		genstruct::AllocatedTable<LBlock *>* lbs = BB_LBLOCKS(bb);
		ASSERT(lbs);
		ilp::Var *var = ipet::VAR(bb);
		ASSERT(var);
		return lbs->count() * int(system->valueOf(var));
	}

	virtual int mergeContext(int v1, int v2) { return v1 + v2; }
	virtual int mergeAgreg(int v1, int v2) { return v1 + v2; }

	void collect(Collector& collector, BasicBlock *bb) {
		if(bb->isEnd())
			return;
		collector.collect(bb->address(), bb->size(), count(bb));
	}

protected:
	virtual int count(BasicBlock *bb) = 0;
	ilp::System *system;

private:
	cstring _name, _unit;
};


// BB hit collector
class HitCollector: public CacheCollector {
public:
	HitCollector(WorkSpace *ws): CacheCollector(ws, "L1 Instruction Cache Hit Number") { }

protected:
	virtual int count(BasicBlock *bb) {
		int sum = 0;
		genstruct::AllocatedTable<LBlock *>* lbs = BB_LBLOCKS(bb);
		ASSERT(lbs);
		for(int i = 0; i < lbs->count(); i++) {
			ilp::Var *bb_var = VAR(bb);
			ASSERT(bb_var);
			ilp::Var *var = MISS_VAR(lbs->get(i));
			ASSERT(var);
			sum +=  int(system->valueOf(bb_var))  - int(system->valueOf(var));
		}
		return sum;
	}
};


// BB miss collector
class MissCollector: public CacheCollector {
public:
	MissCollector(WorkSpace *ws): CacheCollector(ws, "L1 Instruction Cache Miss Number") { }
protected:
	virtual int count(BasicBlock *bb) {
		int sum = 0;
		genstruct::AllocatedTable<LBlock *>* lbs = BB_LBLOCKS(bb);
		ASSERT(lbs);
		for(int i = 0; i < lbs->count(); i++) {
			ilp::Var *bb_var = VAR(bb);
			ASSERT(bb_var);
			ilp::Var *var = MISS_VAR(lbs->get(i));
			ASSERT(var);
			sum +=  int(system->valueOf(var));
		}
		return sum;
	}
};


class NCCollector: public CacheCollector {
public:
	NCCollector(WorkSpace *ws): CacheCollector(ws, "L1 Instruction Cache NC Number") { }
protected:
	virtual int count(BasicBlock *bb) {
		int sum = 0;
		genstruct::AllocatedTable<LBlock *>* lbs = BB_LBLOCKS(bb);
		ASSERT(lbs);
		ilp::Var *xi = VAR(bb);
		ASSERT(xi);
		int xiv = int(system->valueOf(xi));
		for(int i = 0; i < lbs->count(); i++)
			if(otawa::CATEGORY(lbs->get(i)) == otawa::NOT_CLASSIFIED)
				sum += xiv;
		return sum;
	}
};


/**
 * Property giving the variable counting the number of misses.
 *
 * @par Hook
 * @li @ref LBlock
 *
 * @p Feature
 * @li @ref ICACHE_CONSTRAINT2_FEATURE
 */
Identifier<ilp::Var *> MISS_VAR("otawa::MISS_VAR", 0);


/**
 * Ensures that the constraints for cache analysis by categories 2 has been built.
 *
 * @par Properties
 * @li @ref MISS_VAR
 *
 * @par Configuration
 * @li @ref ICACHE_NO_OBJECTIVE
 *
 * @par Processors
 * @li @ref CAT2ConstraintBuilder
 */
p::feature ICACHE_ONLY_CONSTRAINT2_FEATURE("otawa::ICACHE_ONLY_CONSTRAINT2_FEATURE", new Maker<CAT2OnlyConstraintBuilder>());


/**
 * Ensures that the constraints for cache analysis by categories 2 has been built
 * and that the term of instruction cache has been added to the objective function.
 *
 * @par Properties
 * @li @ref MISS_VAR
 *
 * @par Configuration
 * @li @ref ICACHE_NO_OBJECTIVE
 *
 * @par Processors
 * @li @ref CAT2ConstraintBuilder
 */
p::feature ICACHE_CONSTRAINT2_FEATURE("otawa::ICACHE_CONSTRAINT2_FEATURE", new Maker<CAT2ConstraintBuilder>());



/**
 * @class CAT2OnlyConstraintBuilder
 * Build the constraints to support categories of cat2 module.
 *
 * It creates a xmiss for each lblock, and add constraints:
 * ALWAYS_HIT: xmiss = 0
 * ALWAYS_MISS: xmiss = x
 *
 * In the case of FIRST_MISS, we take into account the FIRST_MISS level, and the possibility
 * of Linked lblocks. If LinkedBlocksDetector was not ran, each lblock is treated separately.
 * FIRST_MISS(L): (sum xmiss_for_lblocks_sharing_the_same_cacheblock) <= sum entry_edges_for_loop_L
 *
 * NOT_CLASSIFIED: xmiss <= x
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref ICACHE_CATEGORY2_FEATURE
 * @li @ref DOMINANCE_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref hard::CACHE_CONFIGURATION_FEATURE
 *
 * @par Provided features
 * @li @ref INST_CACHE_SUPPORT_FEATURE
 * @li @ref ICACHE_ONLY_CONSTRAINT2_FEATURE
 *
 * @par Statistics
 * none
 *
 *
 */
p::declare CAT2OnlyConstraintBuilder::reg = p::init("otawa::CAT2OnlyConstraintBuilder", Version(1, 0, 0))
	.maker<CAT2OnlyConstraintBuilder>()
	.require(ASSIGNED_VARS_FEATURE)
	.require(ICACHE_CATEGORY2_FEATURE)
	.require(DOMINANCE_FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.provide(INST_CACHE_SUPPORT_FEATURE)
	.provide(ICACHE_ONLY_CONSTRAINT2_FEATURE);

/**
 */
CAT2OnlyConstraintBuilder::CAT2OnlyConstraintBuilder(p::declare& r): Processor(r) {
}


void CAT2OnlyConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = EXPLICIT(props);
}


/**
 */
void CAT2OnlyConstraintBuilder::collectStats(WorkSpace *ws) {
	recordStat(INST_CACHE_SUPPORT_FEATURE, new HitCollector(ws));
	recordStat(INST_CACHE_SUPPORT_FEATURE, new MissCollector(ws));
	recordStat(INST_CACHE_SUPPORT_FEATURE, new NCCollector(ws));
}


/**
 */
void CAT2OnlyConstraintBuilder::processWorkSpace(otawa::WorkSpace *fw) {
	static const string
		ah_msg = "L1 I$: always hit",
		am_msg = "L1 I$: always miss",
		fm_msg = "L1 I$: first miss",
		nc_msg = "L1 I$: not-classified";
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();
	if(!cache)
		throw ProcessorException(*this, "no instruction cache available");
	ilp::System *system = SYSTEM(fw);
	LBlockSet **lbsets = LBLOCKS(fw);

	// generate the constraints
	for (int i = 0 ; i < cache->rowCount(); i++) {
		for (LBlockSet::Iterator lblock(*lbsets[i]); lblock; lblock++) {
			if ((lblock->id() == 0) || (lblock->id() == (lbsets[i]->count() - 1)))
				continue; /* Skip first / last l-blocks */

			// create x_miss variable
			ilp::Var *miss;
			if(!_explicit)
				miss = system->newVar();
			else {
				StringBuffer buf1;
				buf1 << "x" << lblock->bb()->number() << "_miss_" << lblock->address() << "_" << lblock->id();
				String name1 = buf1.toString();
				miss = system->newVar(name1);
			}
			MISS_VAR(lblock) = miss;

			// add the constraint depending on the lblock category
			switch(cache::CATEGORY(lblock)) {
			case cache::ALWAYS_HIT: {
				// Add constraint: xmiss = 0
				Constraint *cons2 = system->newConstraint(ah_msg, Constraint::EQ,0);
				cons2->addLeft(1, MISS_VAR(lblock));
			}
			break;
			case cache::FIRST_HIT:
			case cache::NOT_CLASSIFIED: {
				// Add constraint: xmiss <= x
				Constraint *cons3 = system->newConstraint(nc_msg, Constraint::LE);
				cons3->addLeft(1, MISS_VAR(lblock));
				cons3->addRight(1, VAR(lblock->bb()));
			}
			break;
			case cache::ALWAYS_MISS: {
				// Add constraint: xmiss = x
				Constraint *cons3 = system->newConstraint(am_msg, Constraint::EQ);
				cons3->addLeft(1, MISS_VAR(lblock));
				cons3->addRight(1, VAR(lblock->bb()));
			}
			break;
			case cache::FIRST_MISS: {
				BasicBlock *header = cache::CATEGORY_HEADER(lblock);
				ASSERT(header != NULL);

				if (LINKED_BLOCKS(lblock) != NULL) {
					/* linked l-blocks first-miss */
					genstruct::Vector<LBlock *> &linked = **LINKED_BLOCKS(lblock);
					/* We add constraints only once per group */
					if (linked[linked.length() - 1] == *lblock) {

						/* Add constraint: (sum of lblock l in list) xmiss_l <= sum of entry-edges of the loop */
						Constraint *cons6 = system->newConstraint(fm_msg, Constraint::LE);
						for (genstruct::Vector<LBlock *>::Iterator iter(linked); iter; iter++) {
							cons6->addLeft(1, MISS_VAR(iter));
						}
						for (BasicBlock::InIterator inedge(header); inedge; inedge++) {
							if (!Dominance::dominates(header, inedge->source())) {
								/* found an entry-edge */
								cons6->addRight(1, VAR(*inedge));
							}
						}
					}

				} else {
					/* standard first-miss */
					/* Add constraint: xmiss <= sum of entry-edges of the loop */
					Constraint *cons5a = system->newConstraint(fm_msg, Constraint::LE);
					cons5a->addLeft(1, MISS_VAR(lblock));
					for (BasicBlock::InIterator inedge(header); inedge; inedge++) {
						if (!Dominance::dominates(header, inedge->source())) {
							/* found an entry-edge */
							cons5a->addRight(1, VAR(*inedge));
						}
					}
				}
				// Add constraint: xmiss <= x
				Constraint *cons1 = system->newConstraint(fm_msg, Constraint::LE);
				cons1->addRight(1, VAR(lblock->bb()));
				cons1->addLeft(1, MISS_VAR(lblock));
			}
			break;

			default:
				ASSERT(false);
				break;
			}
		}
	}
}


/**
 * @class CAT2ConstraintBuilder
 *
 * This processor produces constraints, using the categorization of the L-Blocks,
 * as provided by @ref CAT2OnlyConstraintBuilder and the time spent
 * in cache misses added to the objective function. Added terms have the form, t_miss * x_miss,
 * where t_miss is the miss time and x_miss the number of misses of a block.
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_CONSTRAINT2_FEATURE
 *
 * @par Statistics
 * none
 */
// registration
p::declare CAT2ConstraintBuilder::reg = p::init("otawa::CAT2ConstraintBuilder", Version(1, 1, 0))
	.base(CAT2OnlyConstraintBuilder::reg)
	.maker<CAT2ConstraintBuilder>()
	.require(ipet::OBJECT_FUNCTION_FEATURE)
	.provide(ICACHE_CONSTRAINT2_FEATURE);


/**
 */
CAT2ConstraintBuilder::CAT2ConstraintBuilder(p::declare& r): CAT2OnlyConstraintBuilder(r) {
}


/**
 */
void CAT2ConstraintBuilder::processWorkSpace(otawa::WorkSpace *ws) {
	CAT2OnlyConstraintBuilder::processWorkSpace(ws);

	// get configurations
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(ws)->instCache();
	ilp::System *system = SYSTEM(ws);
	LBlockSet **lbsets = LBLOCKS(ws);
	int penalty = cache->missPenalty();

	// generate the constraints
	for (int i = 0 ; i < cache->rowCount(); i++)
		for (LBlockSet::Iterator lblock(*lbsets[i]); lblock; lblock++)

			// add x_miss * t_miss
			system->addObjectFunction(penalty, MISS_VAR(lblock));
}

}	// otawa

