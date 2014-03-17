/*
 * DataCatConstraintBuilder.cpp
 *
 *  Created on: 12 juil. 2009
 *      Author: casse
 */

#include <otawa/ilp.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/util/Dominance.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/TrivialDataCacheManager.h>
#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/dcache/CatBuilder.h>
#include <otawa/dcache/CatConstraintBuilder.h>
#include <otawa/cache/categories.h>
#include <otawa/ipet/ILPSystemGetter.h>

namespace otawa { namespace dcache {

using namespace otawa::cache;

static SilentFeature::Maker<CatConstraintBuilder> maker;
/**
 * This feature ensures that the constraints associated with each data cache block categories
 * has been translated to ILP constraints and that miss count variables are declared.
 *
 * @p Default processor
 * @li @ref CatConstraintBuilder
 *
 * @p Properties
 * @li @ref MISS_VAR
 * @ingroup dcache
 */
SilentFeature CONSTRAINTS_FEATURE("otawa::dcache::CONSTRAINTS_FEATURE", maker);


/**
 * This property gives the variable counting the number of misses of a BlockAccess
 *
 * @p Hook
 * @li @ref BlockAccess
 * @ingroup dcache
 */
Identifier<ilp::Var *> MISS_VAR("otawa::dcache::MISS_VAR", 0);


/**
 * @class CatConstraintBuilder
 * This processor allocates the variable to count misses of a data cache and
 * makes the constraints for these variables.
 *
 * @p Provided features
 * @li @ref ipet::DATA_CACHE_SUPPORT_FEATURE
 *
 * @p Required features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref dcache::CATEGORY_FEATURE
 * @li @ref DOMINANCE_FEATURE
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 *
 * @p Configuration
 * @li @ref ipet::EXPLICIT
 * @ingroup dcache
 */

p::declare CatConstraintBuilder::reg = p::init("otawa::dcache::CatConstraintBuilder", Version(1, 0, 0))
	.base(Processor::reg)
	.maker<CatConstraintBuilder>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(dcache::CATEGORY_FEATURE)
	.require(DOMINANCE_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.provide(ipet::DATA_CACHE_SUPPORT_FEATURE);


/**
 */
CatConstraintBuilder::CatConstraintBuilder(p::declare& r): Processor(r), _explicit(false) {
}


/**
 */
void CatConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}


/**
 */
void CatConstraintBuilder::processWorkSpace(otawa::WorkSpace *ws) {
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(ws)->dataCache();
	ilp::System *system = ipet::SYSTEM(ws);
	int penalty = cache->missPenalty();
	//LBlockSet **lbsets = LBLOCKS(fw);

	// traverse CFG
	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++) {
		CFG *cfg = cfgs->get(i);
		if(logFor(LOG_CFG))
			log << "\tprocess CFG " << cfg->label() << io::endl;

		// traverse basic blocks
		for(CFG::BBIterator bb(cfg); bb; bb++) {
			if(logFor(LOG_BB))
				log << "\t\tprocess BB " << bb->number()
					<< " (" << ot::address(bb->address()) << ")\n";

			// traverse blocks accesses
			Pair<int, BlockAccess *> ab = DATA_BLOCKS(bb);
			for(int j = 0; j < ab.fst; j++) {
				BlockAccess& b = ab.snd[j];

                // Create x_miss variable
                ilp::Var *miss;
                if(!_explicit)
                        miss = system->newVar();
                else
                        miss = system->newVar(_ << "XMISS_DATA_" << b.instruction()->address() << "_" << j);
                MISS_VAR(b) = miss;

                // Add the constraint depending on the block access category
                switch(dcache::CATEGORY(b)) {
                	case cache::ALWAYS_HIT: { // Add constraint: xmiss = 0
	                		ilp::Constraint *cons2 = system->newConstraint(ilp::Constraint::EQ,0);
    	            		cons2->addLeft(1, miss);
						}
                		break;
					case cache::FIRST_HIT:
					case cache::NOT_CLASSIFIED: { // Add constraint: xmiss <= x
							ilp::Constraint *cons3 = system->newConstraint(ilp::Constraint::LE);
    	            		cons3->addLeft(1, miss);
        	        		cons3->addRight(1, ipet::VAR(bb));
						}
					break;
                	case cache::ALWAYS_MISS: { // Add constraint: xmiss = x
							ilp::Constraint *cons3 = system->newConstraint(ilp::Constraint::EQ);
    	            		cons3->addLeft(1, miss);
        	        		cons3->addRight(1, ipet::VAR(bb));
						}
                		break;
					case cache::FIRST_MISS: {
							BasicBlock *header = dcache::CATEGORY_HEADER(b);
							ASSERT(header);

							// Add constraint: xmiss <= sum of entry-edges of the loop
							ilp::Constraint *cons5a = system->newConstraint(ilp::Constraint::LE);
						 	cons5a->addLeft(1, miss);
						 	for(BasicBlock::InIterator inedge(header); inedge; inedge++)
						 		if (!Dominance::dominates(header, inedge->source()))
						 			cons5a->addRight(1, ipet::VAR(*inedge));

						 	// Add constraint: xmiss <= x
						 	ilp::Constraint *cons1 = system->newConstraint(ilp::Constraint::LE);
							cons1->addRight(1, ipet::VAR(bb));
							cons1->addLeft(1, miss);
						}
						break;

                	default:
                		ASSERT(false);
				break;
                }

			}
		}
	}
}

} }		// otawa::dcache
