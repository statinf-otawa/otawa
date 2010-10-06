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

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa {
	

Identifier<ilp::Var *> HIT_VAR("otawa::HIT_VAR", 0);
Identifier<ilp::Var *> MISS_VAR("otawa::MISS_VAR", 0);
  
/**
 * @class CAT2ConstraintBuilder
 *
 * This processor produces constraints, using the categorization of the lblocks.
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
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref ICACHE_CATEGORY2_FEATURE
 * @li @ref ILP_SYSTEM_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_SUPPORT_FEATURE
 * 
 * @par Statistics
 * none
 */



CAT2ConstraintBuilder::CAT2ConstraintBuilder(void) : Processor("otawa::CAT2ConstraintBuilder", Version(1, 0, 0)), _explicit(false) {
	require(ASSIGNED_VARS_FEATURE);
	require(ICACHE_CATEGORY2_FEATURE);
	require(DOMINANCE_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ILP_SYSTEM_FEATURE);
	provide(INST_CACHE_SUPPORT_FEATURE);
}

void CAT2ConstraintBuilder::configure(const PropList& props) {
        Processor::configure(props);
	_explicit = EXPLICIT(props);
}

void CAT2ConstraintBuilder::setup(otawa::WorkSpace *fw) {
}

                
void CAT2ConstraintBuilder::processWorkSpace(otawa::WorkSpace *fw) {
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();
	ilp::System *system = SYSTEM(fw);
	int penalty = cache->missPenalty();
	LBlockSet **lbsets = LBLOCKS(fw);
	
	
	/* Modify the Object Function */
	for (int i = 0 ; i < cache->rowCount(); i++) {
		for (LBlockSet::Iterator lblock(*lbsets[i]); lblock; lblock++) {
			if ((lblock->id() == 0) || (lblock->id() == (lbsets[i]->count() - 1)))
				continue; /* Skip first / last l-blocks */
			
	                // Create x_miss variable
	                ilp::Var *miss;
	                if(!_explicit)
	                        miss = system->newVar();
	                else {
	                        StringBuffer buf1;
	                        buf1 << "XMISS_ADDR_" << lblock->address() << "_" << lblock->id();
	                        String name1 = buf1.toString();
	                        miss = system->newVar(name1);
	                }
	                MISS_VAR(lblock) = miss;
	                
	                // Add the constraint depending on the lblock category
	                switch(CATEGORY(lblock)) {
	                	case ALWAYS_HIT: {
		                		// Add constraint: xmiss = 0
		                		Constraint *cons2 = system->newConstraint(Constraint::EQ,0);
	    	            		cons2->addLeft(1, MISS_VAR(lblock));
							}
	                		break;
						case FIRST_HIT:
						case NOT_CLASSIFIED: {
		                		// Add constraint: xmiss <= x
		                		Constraint *cons3 = system->newConstraint(Constraint::LE);
	    	            		cons3->addLeft(1, MISS_VAR(lblock));
	        	        		cons3->addRight(1, VAR(lblock->bb()));
							}
						break;						
	                	case ALWAYS_MISS: {
		                		// Add constraint: xmiss = x
		                		Constraint *cons3 = system->newConstraint(Constraint::EQ);
	    	            		cons3->addLeft(1, MISS_VAR(lblock));
	        	        		cons3->addRight(1, VAR(lblock->bb()));
							}
	                		break;
						case FIRST_MISS: {
								BasicBlock *header = CATEGORY_HEADER(lblock);
								assert(header != NULL);
							
								if (LINKED_BLOCKS(lblock) != NULL) {
									/* linked l-blocks first-miss */
									genstruct::Vector<LBlock *> &linked = *LINKED_BLOCKS(lblock);								
									/* We add constraints only once per group */
									if (linked[linked.length() - 1] == *lblock) {
							
										/* Add constraint: (sum of lblock l in list) xmiss_l <= sum of entry-edges of the loop */
										Constraint *cons6 = system->newConstraint(Constraint::LE);
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
							 		Constraint *cons5a = system->newConstraint(Constraint::LE);
							 		cons5a->addLeft(1, MISS_VAR(lblock));	
							 		for (BasicBlock::InIterator inedge(header); inedge; inedge++) {
							 			if (!Dominance::dominates(header, inedge->source())) {
							 				/* found an entry-edge */
							 				cons5a->addRight(1, VAR(*inedge));
						 				}
						 			}
								}						
								// Add constraint: xmiss <= x
								Constraint *cons1 = system->newConstraint(Constraint::LE);
								cons1->addRight(1, VAR(lblock->bb()));
								cons1->addLeft(1, MISS_VAR(lblock));			
							}							
							break;
							
	                	default:
	                		ASSERT(false);
					break;
	            }
	                // Add x_miss*penalty to object function
			system->addObjectFunction(penalty, MISS_VAR(lblock));
		}
	}
}

}
