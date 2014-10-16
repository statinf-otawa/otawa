
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
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/cat2/MUSTPERS.h>
#include <otawa/util/UnrollingListener.h>
#include <otawa/util/DefaultListener.h>

#include <elm/rtti.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace elm;

ENUM(otawa::fmlevel_t)
ENUM_BEGIN(otawa::fmlevel_t)
	VALUE(FML_INNER),
	VALUE(FML_OUTER),
	VALUE(FML_MULTI),
	VALUE(FML_NONE)
ENUM_END

namespace otawa {

template<>
void Identifier<fmlevel_t>::fromString (PropList &props, const string &str) const {
	value_t *val = type_info<fmlevel_t>::values();
	while(val->name()) {
		if(val->name() == str) {
			this->set(props, (fmlevel_t)val->value());
			return;
		}
		val++;
	}
	throw io::IOException("value does not match any enumerated value");
}


/**
 *
 * This feature represents the availability of Abstract Cache State informations.
 *
 * @par Properties
 * @li @ref CACHE_ACS
 *
 * @par Processors
 * @li @ref ACSBuilder (default)
 */
p::feature ICACHE_ACS_FEATURE("otawa::ICACHE_ACS_FEATURE", new Maker<ACSBuilder>());

/**
 * This property represents the "must" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
 Identifier<genstruct::Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST("otawa::CACHE_ACS_MUST", NULL);

/**
 * This property allows us to set an entry must ACS.
 *
 * @par Hooks
 * @li @ref PropList
 */
 Identifier<Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST_ENTRY("otawa::CACHE_ACS_MUST_ENTRY", NULL);



/**
 * This property represents the "persistence" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<genstruct::Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS("otawa::CACHE_ACS_PERS", NULL);

/**
 * This property represents the "persistence" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<bool> PSEUDO_UNROLLING("otawa::PSEUDO_UNROLLING", true);


/**
 * Specify the loop-level-precision of the First Miss computation (inner, outer, multi-level)
 */
Identifier<fmlevel_t> FIRSTMISS_LEVEL("otawa::FIRSTMISS_LEVEL", FML_MULTI);


/**
 * @class ACSBuilder
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

p::declare ACSBuilder::reg = p::init("otawa::ACSBuilder", Version(1, 1, 0))
	.base(Processor::reg)
	.maker<ACSBuilder>()
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.require(ICACHE_FIRSTLAST_FEATURE)
	.provide(ICACHE_ACS_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE);

ACSBuilder::ACSBuilder(p::declare& r) : Processor(r), must_entry(0), level(FML_NONE), unrolling(false) {
}


void ACSBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset, const hard::Cache *cache) {

	int line = lbset->line();
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
			UnrollingListener<MUSTProblem> mustList(fw, mustProb);
			FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > mustFp(mustList);
			HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(NULL, must_entry ? must_entry->get(line) : NULL);


			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++)
					CACHE_ACS_MUST(bb)->add(new MUSTProblem::Domain(*mustList.results[cfg->number()][bb->number()]));

		} else {
			DefaultListener<MUSTProblem> mustList(fw, mustProb);
			DefaultFixPoint<DefaultListener<MUSTProblem> > mustFp(mustList);
			HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(NULL, must_entry ? must_entry->get(line) : NULL);


			/* Store the resulting ACS into the properties */
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++)
					CACHE_ACS_MUST(bb)->add(new MUSTProblem::Domain(*mustList.results[cfg->number()][bb->number()]));
		}
	} else {
		if (unrolling) {
			/* Do combined MUST/PERS analysis */
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
			UnrollingListener<MUSTPERS> mustpersList( fw, mustpers);
			FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > mustpersFp(mustpersList);
			HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();

			/* Store. */
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUSTProblem::Domain &must= mustpersList.results[cfg->number()][bb->number()]->getMust();
					PERSProblem::Domain &pers= mustpersList.results[cfg->number()][bb->number()]->getPers();
					CACHE_ACS_MUST(bb)->add(new MUSTProblem::Domain(must));
					CACHE_ACS_PERS(bb)->add(new PERSProblem::Domain(pers));

				}
		} else {

			/* Do combined MUST/PERS analysis */
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
			DefaultListener<MUSTPERS> mustpersList( fw, mustpers);
			DefaultFixPoint<DefaultListener<MUSTPERS> > mustpersFp(mustpersList);
			HalfAbsInt<DefaultFixPoint<DefaultListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			mustHai.solve();

			/* Store. */
			for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
				for (CFG::BBIterator bb(cfg); bb; bb++) {
					MUSTProblem::Domain &must= mustpersList.results[cfg->number()][bb->number()]->getMust();
					PERSProblem::Domain &pers= mustpersList.results[cfg->number()][bb->number()]->getPers();
					CACHE_ACS_MUST(bb)->add(new MUSTProblem::Domain(must));
					CACHE_ACS_PERS(bb)->add(new PERSProblem::Domain(pers));
			}
		}
	}
}

void ACSBuilder::configure(const PropList &props) {
	Processor::configure(props);
	level = FIRSTMISS_LEVEL(props);
	unrolling = PSEUDO_UNROLLING(props);
	must_entry = CACHE_ACS_MUST_ENTRY(props);
}

void ACSBuilder::processWorkSpace(WorkSpace *fw) {
	//int i;

	FIRSTMISS_LEVEL(fw) = level;
	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++) {
			CACHE_ACS_MUST(bb) = new genstruct::Vector<MUSTProblem::Domain*>;
			if (level != FML_NONE)
				CACHE_ACS_PERS(bb) = new genstruct::Vector<PERSProblem::Domain*>;
		}
	}

	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();

	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(fw, lbsets[i], cache);
	}
}

}
