#include <stdio.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>

#include <otawa/cache/cat2/MAYProblem.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/ACSMayBuilder.h>
#include <otawa/util/UnrollingListener.h>
#include <otawa/util/DefaultListener.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace otawa::util;
using namespace elm;




namespace otawa {

/**
 * 
 * This feature represents the availability of MAY Abstract Cache State informations.
 * 
 * @par Properties
 * @li @ref CACHE_ACS
 */
Feature<ACSMayBuilder> ICACHE_ACS_MAY_FEATURE("otawa.cache.acsmayfeature");

/**
 * This property represents the "may" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock 
 */
 Identifier<genstruct::Vector<MAYProblem::Domain*>* > CACHE_ACS_MAY("otawa::cache_acs_may", NULL);

/**
 * This property allows us to set an entry may ACS. 
 *
 * @par Hooks
 * @li @ref PropList 
 */
 Identifier<Vector<MAYProblem::Domain*>* > CACHE_ACS_MAY_ENTRY("otawa::cache_acs_may_entry", NULL);
 

/**
 * @class ACSMayBuilder
 *
 * This processor produces the Abstract Cache States (ACS), for the May and Persistence problems.
 *
 * @par Configuration
 * @li @ref FIRSTMISS_LEVEL identifier determines the First Miss method (FML_OUTER, FML_INNER, FML_MULTI, FML_NONE). FML_MULTI is the default.
 * @li @ref PSEUDO_UNROLLIG identifier determines if we do the Pseudo-Unrolling while doing the abstract interpretation.
 *
 * @par Required features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref ICACHE_FIRSTLAST_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_ACS_FEATURE
 * 
 * @par Statistics
 * none
 */

ACSMayBuilder::ACSMayBuilder(void) : Processor("otawa::ACSMayBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_FIRSTLAST_FEATURE);
	provide(ICACHE_ACS_FEATURE);
}


void ACSMayBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset, const hard::Cache *cache) {

	int line = lbset->line();	
	/* 
	 * Solve the problem for the current cache line:
	 * Now that the first/last lblock are detected, execute the analysis. 
	 */			

#ifdef DEBUG
	cout << "[TRACE] Doing line " << line << "\n";
#endif
	MAYProblem mayProb(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
	if (unrolling) {			
		UnrollingListener<MAYProblem> mayList(fw, mayProb);
		FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > mayFp(mayList);
		util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > > mayHai(mayFp, *fw);
		mayHai.solve(NULL, may_entry ? may_entry->get(line) : NULL);
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				CACHE_ACS_MAY(bb)->add(new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));
	
	} else {
		DefaultListener<MAYProblem> mayList(fw, mayProb);
		DefaultFixPoint<DefaultListener<MAYProblem> > mayFp(mayList);
		util::HalfAbsInt<DefaultFixPoint<DefaultListener<MAYProblem> > > mayHai(mayFp, *fw);
		mayHai.solve(NULL, may_entry ? may_entry->get(line) : NULL);
		/* Store the resulting ACS into the properties */
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				CACHE_ACS_MAY(bb)->add(new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));
	}
}

void ACSMayBuilder::configure(const PropList &props) {
	Processor::configure(props);
	unrolling = PSEUDO_UNROLLING(props);
	may_entry = CACHE_ACS_MAY_ENTRY(props);
}

void ACSMayBuilder::processWorkSpace(WorkSpace *fw) {
	int i;
	
	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++)
			CACHE_ACS_MAY(bb) = new genstruct::Vector<MAYProblem::Domain*>;
	}
	
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
				
	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(fw, lbsets[i], cache);	
	}	
}

}
