

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

#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/ACSMayBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/cache/cat2/CAT2Builder.h>
#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/MAYProblem.h>
using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;


namespace otawa {
	
/**
 *
 * In the case of a FIRST_HIT (or FIRST_MISS) property, contains the header
 * of the loop causing the HIT (or MISS) at the first iteration.
 *
 * @par Hooks
 * @li @ref LBlocks
 */
Identifier<BasicBlock*> CATEGORY_HEADER("otawa::category_header", 0);


/**
 * @class CAT2Builder
 *
 * This processor produces categorization information for each l-block
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
 * @li @ref ICACHE_CATEGORY_FEATURE
 * 
 * @par Statistics
 * none
 */

CAT2Builder::CAT2Builder(void) : CFGProcessor("otawa::CAT2Builder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_ACS_FEATURE);
	require(ICACHE_FIRSTLAST_FEATURE);
	provide(ICACHE_CATEGORY_FEATURE);
}

void CAT2Builder::processLBlockSet(otawa::CFG *cfg, LBlockSet *lbset, const hard::Cache *cache) {
	int line = lbset->line();
	static double moypr = 0;
	static double moy = 0;
	
	/* Use the results to set the categorization */
	for (LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		if ((lblock->id() == 0) || (lblock->id() == lbset->count() - 1))
			continue;
			
		if (LBLOCK_ISFIRST(lblock)) {
			MUSTProblem::Domain *must = CACHE_ACS_MUST(lblock->bb())->get(line);
			MAYProblem::Domain *may = NULL;
			if (CACHE_ACS_MAY(lblock->bb()) != NULL)
				may = CACHE_ACS_MAY(lblock->bb())->get(line);
			BasicBlock *header;
			if (may) {
				CATEGORY(lblock) = NOT_CLASSIFIED;
			} else {
				CATEGORY(lblock) = ALWAYS_MISS;
			}
			
			if (must->contains(lblock->cacheblock())) {
				CATEGORY(lblock) = ALWAYS_HIT;
			} else if (may && !may->contains(lblock->cacheblock())) {
				CATEGORY(lblock) = ALWAYS_MISS;
			} else if (firstmiss_level != FML_NONE) {
				if (Dominance::isLoopHeader(lblock->bb()))
					header = lblock->bb();
			  	else header = ENCLOSING_LOOP_HEADER(lblock->bb());
			  	
			  	int bound;
			  	bool perfect_firstmiss = true;										
				PERSProblem::Domain *pers = CACHE_ACS_PERS(lblock->bb())->get(line);
				bound = 0;
				
				if ((pers->length() > 1) && (firstmiss_level == FML_INNER))
					bound = pers->length() - 1;
				CATEGORY_HEADER(lblock) = NULL;		
			  	for (int k = pers->length() - 1 ; (k >= bound) && (header != NULL); k--) {
					if (pers->isPersistent(lblock->cacheblock(), k)) {
						CATEGORY(lblock) = FIRST_MISS;
						CATEGORY_HEADER(lblock) = header;
					} else perfect_firstmiss = false;
					header = ENCLOSING_LOOP_HEADER(header);
				}
			
				if ((firstmiss_level == FML_OUTER) && (perfect_firstmiss == false))
					CATEGORY(lblock) = ALWAYS_MISS;																				
			} /* of category condition test */			
		} else {
			CATEGORY(lblock) = ALWAYS_MISS;
		} 
	}
	

}

void CAT2Builder::setup(WorkSpace *fw) {
}

void CAT2Builder::configure(const PropList &props) {
	CFGProcessor::configure(props);
	firstmiss_level = FIRSTMISS_LEVEL(props);
}

void CAT2Builder::processCFG(otawa::WorkSpace *fw, otawa::CFG *cfg) {
	int i;
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
	
	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(cfg, lbsets[i], cache );
	}	
}

}
