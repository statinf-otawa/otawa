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

namespace otawa { namespace dcache {


Identifier<ilp::Var *> HIT_VAR("otawa::dcache::HIT_VAR", 0);
Identifier<ilp::Var *> MISS_VAR("otawa::dcache::MISS_VAR", 0);

CatConstraintBuilder::CatConstraintBuilder(void)
: Processor("otawa::dcache::CatConstraintBuilder", Version(1, 0, 0)), _explicit(false) {
	require(ipet::ASSIGNED_VARS_FEATURE);
	require(dcache::CACHE_CATEGORY_FEATURE);
	require(DOMINANCE_FEATURE);
	//require(ILP_SYSTEM_FEATURE);
	provide(ipet::DATA_CACHE_SUPPORT_FEATURE);
}

void CatConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}

void CatConstraintBuilder::setup(otawa::WorkSpace *fw) {
}


void CatConstraintBuilder::processWorkSpace(otawa::WorkSpace *ws) {
	const hard::Cache *cache = ws->platform()->cache().dataCache();
	ilp::System *system = ipet::SYSTEM(ws);
	int penalty = cache->missPenalty();
	//LBlockSet **lbsets = LBLOCKS(fw);

	// traverse CFG
	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++) {
		CFG *cfg = cfgs->get(i);

		// traverse basic blocks
		for(CFG::BBIterator bb(cfg); bb; bb++) {
			cerr << "\tBB " << bb->number() << " (" << bb->address() << ")\n";

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
                switch(CATEGORY(b)) {
                	case ALWAYS_HIT: { // Add constraint: xmiss = 0
	                		ilp::Constraint *cons2 = system->newConstraint(ilp::Constraint::EQ,0);
    	            		cons2->addLeft(1, miss);
						}
                		break;
					case FIRST_HIT:
					case NOT_CLASSIFIED: { // Add constraint: xmiss <= x
							ilp::Constraint *cons3 = system->newConstraint(ilp::Constraint::LE);
    	            		cons3->addLeft(1, miss);
        	        		cons3->addRight(1, ipet::VAR(bb));
						}
					break;
                	case ALWAYS_MISS: { // Add constraint: xmiss = x
							ilp::Constraint *cons3 = system->newConstraint(ilp::Constraint::EQ);
    	            		cons3->addLeft(1, miss);
        	        		cons3->addRight(1, ipet::VAR(bb));
						}
                		break;
					case FIRST_MISS: {
							BasicBlock *header = CATEGORY_HEADER(b);
							ASSERT(header != NULL);

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
