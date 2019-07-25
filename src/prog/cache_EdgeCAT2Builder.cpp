

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

#include <otawa/cache/cat2/EdgeACSBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/cache/cat2/EdgeCAT2Builder.h>
#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/MAYProblem.h>
using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;


namespace otawa {
	

/**
 * @class EdgeCAT2Builder
 *
 * This processor produces categorization information for each l-block.
 * 
 * For each lblock: 
 * If the cache block exists in the MUST ACS, then the l-block is ALWAYS_HIT
 * If the cache block exists in the PERS ACS, then the block is FIRST_MISS
 * If we performed the MAY ACS computation, and the cache block is not in MAY ACS, the block is ALWAYS_MISS
 * Otherwise the lblock is NOT_CLASSIFIED.
 * 
 * If the Multi-Level persistence was computed, then the FIRST_MISS level is computed as follow: 
 * We iterate over the Items of the PERS ACS, from inner to outer 
 * The first Pers Item for which the lblock is not persistent determines the FIRST_MISS level. 
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref ICACHE_ACS_FEATURE
 * @li @ref ICACHE_FIRSTLAST_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_CATEGORY2_FEATURE
 * 
 * @par Statistics
 * none
 */

EdgeCAT2Builder::EdgeCAT2Builder(void) : CFGProcessor("otawa::EdgeCAT2Builder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_EDGE_ACS_FEATURE);
	require(ICACHE_FIRSTLAST_FEATURE);
	provide(ICACHE_EDGE_CATEGORY2_FEATURE);
}

void EdgeCAT2Builder::processLBlockSet(otawa::CFG *cfg, LBlockSet *lbset, const hard::Cache *cache) {
	unsigned int line = lbset->line();
	/*static double moypr = 0;
	static double moy = 0;*/
	
	/* Use the results to set the categorization */
	for (LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		if ((lblock->id() == 0) || (lblock->id() == lbset->count() - 1))
			continue;
			
	
		Vector<MUSTProblem::Domain*> &mustVec = *CACHE_EDGE_ACS_MUST(lblock);
		Vector<PERSProblem::Domain*> &persVec = *CACHE_EDGE_ACS_PERS(lblock);
		
		CATEGORY_EDGE(lblock) = new Vector<category_t>();
		CATEGORY_EDGE_HEADER(lblock) = new Vector<BasicBlock*>();

		BasicBlock::InIterator inedge(lblock->bb());
		for (int i = 0; i < mustVec.length(); i++) {
			
			MUSTProblem::Domain *must = mustVec[i];
			
			PERSProblem::Domain *pers = NULL;
			if (firstmiss_level != FML_NONE) {
				pers = persVec[i];
				/* enter/leave context if in-edge enters or exits loops */
				if (LOOP_EXIT_EDGE(inedge)) {
					/* find loop header of inner-most exited loop */
					BasicBlock *header = inedge->source();
					if (!LOOP_HEADER(header))
						header = ENCLOSING_LOOP_HEADER(header);
					ASSERT(header && LOOP_HEADER(header));
	
					/* now leave contexts for each loop between header and LOOP_EXIT_EDGE(inedge) (included) */
					pers->leaveContext();
					while (header != LOOP_EXIT_EDGE(inedge)) {	
						pers->leaveContext();
						header = ENCLOSING_LOOP_HEADER(header);
						ASSERT(header);
					} 
				}
				if (LOOP_HEADER(lblock->bb()) && !BACK_EDGE(inedge)) {
					/* an entry edge may not enter more than one loop */
					pers->enterContext();
				}
			}
			
			
			BasicBlock *header = NULL;	
			category_t cat = NOT_CLASSIFIED;	
			BasicBlock *cat_header = NULL;		
				
			if (must->contains(lblock->cacheblock())) {
				cat = ALWAYS_HIT;
			} else if (firstmiss_level != FML_NONE) {
				if (Dominance::isLoopHeader(lblock->bb()))
					header = lblock->bb();
			  	else header = ENCLOSING_LOOP_HEADER(lblock->bb());
			  	
			  	int bound;
			  	bool perfect_firstmiss = true;										
				
				bound = 0;
				
				if ((pers->length() > 1) && (firstmiss_level == FML_INNER))
					bound = pers->length() - 1;
				cat_header = NULL;		
			  	for (int k = pers->length() - 1 ; (k >= bound) && (header != NULL); k--) {
					if (pers->isPersistent(lblock->cacheblock(), k)) {
						cat = FIRST_MISS;
						cat_header = header;
					} else perfect_firstmiss = false;
					header = ENCLOSING_LOOP_HEADER(header);
				}
			
				if ((firstmiss_level == FML_OUTER) && (perfect_firstmiss == false))
					cat = ALWAYS_MISS;																				
			} /* of category condition test */			
			CATEGORY_EDGE(lblock)->add(cat);
			CATEGORY_EDGE_HEADER(lblock)->add(cat_header);
			inedge++;
		}
	}
	

}

void EdgeCAT2Builder::setup(WorkSpace *fw) {
}

void EdgeCAT2Builder::configure(const PropList &props) {
	CFGProcessor::configure(props);
	firstmiss_level = FIRSTMISS_LEVEL(props);
}

void EdgeCAT2Builder::processCFG(otawa::WorkSpace *fw, otawa::CFG *cfg) {
	//int i;
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
	
	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(cfg, lbsets[i], cache );
	}	
}

  Feature<EdgeCAT2Builder> ICACHE_EDGE_CATEGORY2_FEATURE("otawa::ICACHE_EDGE_CATEGORY2_FEATURE");
  Identifier<Vector<category_t> *> CATEGORY_EDGE("otawa::cactegory_edge",  NULL );
  Identifier<Vector<BasicBlock*> *> CATEGORY_EDGE_HEADER("otawa::category_edge_header", NULL);

}
