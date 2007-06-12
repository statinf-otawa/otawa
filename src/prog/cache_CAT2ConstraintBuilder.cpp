
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
	

Identifier<ilp::Var *> HIT_VAR("otawa::hit_var", 0, otawa::NS);
Identifier<ilp::Var *> MISS_VAR("otawa::miss_var", 0, otawa::NS);
  
/**
 * @class CAT2ConstraintBuilder
 *
 * This processor produces constraints for the categorization of l-blocks.
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref ICACHE_CATEGORY_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_SUPPORT_FEATURE
 * 
 * @par Statistics
 * none
 */



CAT2ConstraintBuilder::CAT2ConstraintBuilder(void) : Processor("otawa::CAT2ConstraintBuilder", Version(1, 0, 0)), _explicit(false) {
	require(ASSIGNED_VARS_FEATURE);
	require(ICACHE_CATEGORY_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	provide(INST_CACHE_SUPPORT_FEATURE);
}

void CAT2ConstraintBuilder::configure(const PropList& props) {
        Processor::configure(props);
	_explicit = EXPLICIT(props);
}

void CAT2ConstraintBuilder::setup(otawa::WorkSpace *fw) {
}

                
void CAT2ConstraintBuilder::processWorkSpace(otawa::WorkSpace *fw) {
	const hard::Cache *cache = fw->platform()->cache().instCache();
	ilp::System *system = getSystem(fw, ENTRY_CFG(fw));
	int penalty = cache->missPenalty();
	LBlockSet **lbsets = LBLOCKS(fw);
	
	
	/* Modify the Object Function */
	for (int i = 0 ; i < cache->lineCount(); i++) {
		for (LBlockSet::Iterator lblock(*lbsets[i]); lblock; lblock++) {
			if ((lblock->id() == 0) || (lblock->id() == (lbsets[i]->count() - 1)))
				continue; /* Skip first / last l-blocks */
			
	                // Create x_miss variable
	                ilp::Var *miss;
	                if(!_explicit)
	                        miss = system->newVar();
	                else {
	                        StringBuffer buf1;
	                        buf1 << "xmiss_" << lblock->address() << "," << i << "," << lblock->id() << ","<< lblock->bb()->number();
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
					break;
	            }
	                // Add x_miss*penalty to object function
			system->addObjectFunction(penalty, MISS_VAR(lblock));
		}
	}
}

}
