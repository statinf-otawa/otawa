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

#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/cache/cat2/PERSProblem.h>
#include <otawa/cache/cat2/EdgeACSBuilder.h>
#include <otawa/cache/cat2/MUSTPERS.h>
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
 * This feature represents the availability of Abstract Cache State informations.
 * 
 * @par Properties
 * @li @ref CACHE_ACS
 */
Feature<EdgeACSBuilder> ICACHE_EDGE_ACS_FEATURE("otawa::ICACHE_EDGE_ACS_FEATURE");

/**
 * This property represents the "must" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock 
 */
 Identifier<Vector<MUSTProblem::Domain*> *> CACHE_EDGE_ACS_MUST("otawa::cache_edge_acs_must", NULL);


 
/**
 * This property represents the "persistence" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock 
 */
Identifier<Vector<PERSProblem::Domain*> *> CACHE_EDGE_ACS_PERS("otawa::cache_edge_acs_pers", NULL);



/**
 * @class EdgeACSBuilder
 *
 * This processor builds the MUST and PERS cache states before each basic block.
 * The MUST cache state lists the ID of cache blocks which must be in the cache and is useful to determine
 * ALWAYS_HIT blocks.
 * The PERS cache state lists the ID of cache blocks which may be in the cache, but cannot be replaced once
 * they have been loaded. It is useful to determine the FIRST_MISS blocks.
 * 
 * The Persistence can be computed in 3 ways:
 * - Outer: A block is FIRST_MISS if it can not be replaced within the whole program
 * - Inner: A block is FIRST_MISS if it can not be replaced from the inner-most loop containing it.
 * - Multi: The FIRST_MISS is parametrized by a variable L, representing the outer-most loop whose execution does not replace the block. 
 *
 * The analysis can be used with Pseudo-Unrolling:
 * In this case, unroll (using FirstUnrollingFixPoint with HalfAbsInt) the first iteration of each loop during abstract interpretation.
 * But the ACS corresponding to multiple iterations of a basic block are merged back into one ACS at the end of the analysis.
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

EdgeACSBuilder::EdgeACSBuilder(void) : Processor("otawa::EdgeACSBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_FIRSTLAST_FEATURE);
	provide(ICACHE_EDGE_ACS_FEATURE);
}


#define STORE_MUST \
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) { \
		for (CFG::BBIterator bb(cfg); bb; bb++) { \
			AllocatedTable<LBlock*> &lblocks = *BB_LBLOCKS(bb); \
			if (BB_LBLOCKS(bb) != NULL) { \
				Vector<MUSTProblem::Domain *> must; \
				for (BasicBlock::InIterator inedge(bb); inedge; inedge++) \
					must.add(mustList.results_out[cfg->number()][inedge->source()->number()]); \
				for (int i = 0; i < lblocks.count(); i++) { \
					if (cache->line(lblocks[i]->address()) == line) { \
						int pred = 0; \
						CACHE_EDGE_ACS_MUST(lblocks[i]) = new Vector<MUSTProblem::Domain*>(); \
						for (BasicBlock::InIterator inedge(bb); inedge; inedge++) { \
							CACHE_EDGE_ACS_MUST(lblocks[i])->add(new MUSTProblem::Domain(*must[pred])); \
							mustProb.updateLBlock(*must[pred], lblocks[i]); \
							pred++; \
						} \
					} \
				} \
			} \
		} \
	}
	
#define STORE_MUSTPERS \
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) { \
		for (CFG::BBIterator bb(cfg); bb; bb++) { \
			AllocatedTable<LBlock*> &lblocks = *BB_LBLOCKS(bb); \
			if (BB_LBLOCKS(bb) != NULL) { \
				Vector<MUSTPERS::Domain *> mustpersAcs; \
				for (BasicBlock::InIterator inedge(bb); inedge; inedge++) \
					mustpersAcs.add(mustpersList.results_out[cfg->number()][inedge->source()->number()]); \
				for (int i = 0; i < lblocks.count(); i++) { \
					if (cache->line(lblocks[i]->address()) == line) { \
						int pred = 0; \
						CACHE_EDGE_ACS_MUST(lblocks[i]) = new Vector<MUSTProblem::Domain*>(); \
						CACHE_EDGE_ACS_PERS(lblocks[i]) = new Vector<PERSProblem::Domain*>(); \
						for (BasicBlock::InIterator inedge(bb); inedge; inedge++) { \
							CACHE_EDGE_ACS_MUST(lblocks[i])->add(new MUSTProblem::Domain(mustpersAcs[pred]->getMust())); \
							CACHE_EDGE_ACS_PERS(lblocks[i])->add(new PERSProblem::Domain(mustpersAcs[pred]->getPers())); \
							mustpers.updateLBlock(*mustpersAcs[pred], lblocks[i]); \
							pred++; \
						} \
					} \
				} \
			} \
		} \
	}
	
	
void EdgeACSBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset, const hard::Cache *cache) {

	unsigned int line = lbset->line();	
	/* 
	 * Solve the problem for the current cache line:
	 * Now that the first/last lblock are detected, execute the analysis. 
	 */			

#ifdef DEBUG
	cout << "[TRACE] Doing line " << line << "\n";
#endif
	if (level == FML_NONE) {
		/* do only the MUST */
		MUSTProblem mustProb(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
		
		
		if (unrolling) {			
			UnrollingListener<MUSTProblem> mustList(fw, mustProb, true);
			FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(NULL, must_entry ? must_entry->get(line) : NULL);			
			STORE_MUST
												
		} else {
			DefaultListener<MUSTProblem> mustList(fw, mustProb, true);
			DefaultFixPoint<DefaultListener<MUSTProblem> > mustFp(mustList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(NULL, must_entry ? must_entry->get(line) : NULL);
			STORE_MUST
		}

	} else {
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
		if (unrolling) {
			/* Do combined MUST/PERS analysis */
			
			UnrollingListener<MUSTPERS> mustpersList( fw, mustpers, true);
			FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > mustpersFp(mustpersList);
			util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();
			STORE_MUSTPERS

		} else {
					
			/* Do combined MUST/PERS analysis */
		
			DefaultListener<MUSTPERS> mustpersList( fw, mustpers, true);
			DefaultFixPoint<DefaultListener<MUSTPERS> > mustpersFp(mustpersList);
			util::HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();
			STORE_MUSTPERS

	
		}
		
	}
	
}

void EdgeACSBuilder::configure(const PropList &props) {
	Processor::configure(props);
	level = FIRSTMISS_LEVEL(props);
	unrolling = PSEUDO_UNROLLING(props);
	must_entry = CACHE_ACS_MUST_ENTRY(props);
}

void EdgeACSBuilder::processWorkSpace(WorkSpace *fw) {
	//int i;
	
	FIRSTMISS_LEVEL(fw) = level;
	// Build the vectors for receiving the ACS...
	/*
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++) {
			AllocatedTable<LBlock*> &lblocks = *BB_LBLOCKS(bb);
			if (BB_LBLOCKS(bb) != NULL) {
				for (int i = 0; i < lblocks.count(); i++) {
					CACHE_ACS_MUST(bb) = new genstruct::Vector<MUSTProblem::Domain*>;
					if (level != FML_NONE)
						CACHE_ACS_PERS(bb) = new genstruct::Vector<PERSProblem::Domain*>;
				}
			}
		}
	}
	*/
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
				
	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(fw, lbsets[i], cache);	
	}	
}

}
