/*
 *	ACSBuilder processor implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <elm/io.h>
#include <elm/data/Vector.h>
#include <elm/rtti.h>

#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/FirstLastBuilder.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cache/cat2/MUSTPERS.h>
#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/PERSProblem.h>
#include <otawa/cfg.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/dfa/hai/DefaultListener.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/cache/LBlockBuilder.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/hai/UnrollingListener.h>


using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace elm;

static rtti::Enum fmtlevel_type(rtti::Enum::make("otawa::fmtlevel_t")
	.value("FML_INNER", FML_INNER)
	.value("FML_OUTER", FML_OUTER)
	.value("FML_MULTI", FML_MULTI)
	.value("FML_NONE", FML_NONE));

namespace otawa {

template<>
void Identifier<fmlevel_t>::fromString (PropList &props, const string &str) const {
	int val = fmtlevel_type.valueFor(str);
	if(val >= 0)
		set(props, fmlevel_t(val));
	else
		throw io::IOException("value does not match any enumerated value");
}


/**
 *
 * This feature represents the availability of Abstract Cache State informations.
 *
 * @par Configuration
 * @li @ref CACHE_ACS_MUST_ENTRY
 *
 * @par Properties
 * @li @ref CACHE_ACS_MUST
 * @li @ref CACHE_ACS_PERS
 *
 * @par Processors
 * @li @ref ACSBuilder (default)
 */
p::feature ICACHE_ACS_FEATURE("otawa::ICACHE_ACS_FEATURE", new Maker<ACSBuilder>());

/**
 * This property represents the MUST Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST("otawa::CACHE_ACS_MUST", 0);

/**
 * This property allows to set an entry MUST ACS. It must be set in the property list passed
 * to the invocation of a code processor.
 *
 * @par Hooks
 * @li @ref PropList (code processor configuration)
 */
Identifier<Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST_ENTRY("otawa::CACHE_ACS_MUST_ENTRY", 0);

/**
 * This property allows to set an entry PERS ACS. It must be set in the property list passed
 * to the invocation of a code processor.
 *
 * @par Hooks
 * @li @ref PropList (code processor configuration)
 */
Identifier<Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS_ENTRY("otawa::CACHE_ACS_PERS_ENTRY", 0);

/**
 * This property represents the "persistence" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS("otawa::CACHE_ACS_PERS", 0);

/**
 * This property represents the "persistence" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<bool> PSEUDO_UNROLLING("otawa::PSEUDO_UNROLLING", false);


/**
 * Specify the loop-level-precision of the First Miss computation (inner, outer, multi-level)
 */
Identifier<fmlevel_t> FIRSTMISS_LEVEL("otawa::FIRSTMISS_LEVEL", FML_MULTI);


/**
 */
otawa::Output& operator<<(otawa::Output& out, const fmlevel_t &fml) {
	switch (fml) {
	case FML_INNER:
		out << "FML_INNER";
		break;
	case FML_OUTER:
		out << "FML_OUTER";
		break;
	case FML_MULTI:
		out << "FML_MULTI";
		break;
	case FML_NONE:
		out << "FML_NONE";
		break;
	default:
		out << "FML_UNKNOWN";
		break;
	}
	return out;
}


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
 * @li @ref PSEUDO_UNROLLING identifier determines if we do the Pseudo-Unrolling while doing the abstract interpretation.
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


/**
 */
p::declare ACSBuilder::reg = p::init("otawa::ACSBuilder", Version(1, 1, 0))
	.base(Processor::reg)
	.maker<ACSBuilder>()
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(cache::COLLECTED_LBLOCKS_FEATURE)
	.require(ICACHE_FIRSTLAST_FEATURE)
	.provide(ICACHE_ACS_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE);

/**
 */
ACSBuilder::ACSBuilder(p::declare& r) : Processor(r), level(FML_NONE), unrolling(false), must_entry(0), pers_entry(0) {
}


/**
 */
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
			dfa::hai::UnrollingListener<MUSTProblem> mustList(fw, mustProb);
			dfa::hai::FirstUnrollingFixPoint<dfa::hai::UnrollingListener<MUSTProblem> > mustFp(mustList);
			dfa::hai::HalfAbsInt<dfa::hai::FirstUnrollingFixPoint<dfa::hai::UnrollingListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(0, must_entry ? must_entry->get(line) : 0);


			for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++)
				for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
					CACHE_ACS_MUST(*bb)->add(new MUSTProblem::Domain(*mustList.results[cfg->index()][bb->index()]));

		} else {
			dfa::hai::DefaultListener<MUSTProblem> mustList(fw, mustProb);
			dfa::hai::DefaultFixPoint<dfa::hai::DefaultListener<MUSTProblem> > mustFp(mustList);
			dfa::hai::HalfAbsInt<dfa::hai::DefaultFixPoint<dfa::hai::DefaultListener<MUSTProblem> > > mustHai(mustFp, *fw);
			mustHai.solve(0, must_entry ? must_entry->get(line) : 0);


			/* Store the resulting ACS into the properties */
			for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++)
				for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
					CACHE_ACS_MUST(*bb)->add(new MUSTProblem::Domain(*mustList.results[cfg->index()][bb->index()]));
		}
	} else {
		if (unrolling) {
			/* Do combined MUST/PERS analysis */
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
			dfa::hai::UnrollingListener<MUSTPERS> mustpersList( fw, mustpers);
			dfa::hai::FirstUnrollingFixPoint<dfa::hai::UnrollingListener<MUSTPERS> > mustpersFp(mustpersList);
			dfa::hai::HalfAbsInt<dfa::hai::FirstUnrollingFixPoint<dfa::hai::UnrollingListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			MUSTPERS::Domain entry(
				must_entry ? *must_entry->get(line) : mustpers.entry().getMust(),
				pers_entry ? *pers_entry->get(line) : mustpers.entry().getPers());
			mustHai.solve(0, &entry, 0);

			/* Store. */
			for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++)
				for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
					const MUSTProblem::Domain &must= mustpersList.results[cfg->index()][bb->index()]->getMust();
					const PERSProblem::Domain &pers= mustpersList.results[cfg->index()][bb->index()]->getPers();
					CACHE_ACS_MUST(*bb)->add(new MUSTProblem::Domain(must));
					CACHE_ACS_PERS(*bb)->add(new PERSProblem::Domain(pers));

				}
		} else {

			/* Do combined MUST/PERS analysis */
			MUSTPERS mustpers(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());
			dfa::hai::DefaultListener<MUSTPERS> mustpersList( fw, mustpers);
			dfa::hai::DefaultFixPoint<dfa::hai::DefaultListener<MUSTPERS> > mustpersFp(mustpersList);
			dfa::hai::HalfAbsInt<dfa::hai::DefaultFixPoint<dfa::hai::DefaultListener<MUSTPERS> > > mustHai(mustpersFp, *fw);
			MUSTPERS::Domain entry(
				must_entry ? *must_entry->get(line) : mustpers.entry().getMust(),
				pers_entry ? *pers_entry->get(line) : mustpers.entry().getPers());
			mustHai.solve(0, &entry, 0);

			/* Store. */
			for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++)
				for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
					const MUSTProblem::Domain &must= mustpersList.results[cfg->index()][bb->index()]->getMust();
					const PERSProblem::Domain &pers= mustpersList.results[cfg->index()][bb->index()]->getPers();
					CACHE_ACS_MUST(*bb)->add(new MUSTProblem::Domain(must));
					CACHE_ACS_PERS(*bb)->add(new PERSProblem::Domain(pers));
			}
		}
	}
}


/**
 */
void ACSBuilder::configure(const PropList &props) {
	Processor::configure(props);
	level = FIRSTMISS_LEVEL(props);
	unrolling = PSEUDO_UNROLLING(props);
	must_entry = CACHE_ACS_MUST_ENTRY(props);
	pers_entry = CACHE_ACS_PERS(props);
}

void ACSBuilder::processWorkSpace(WorkSpace *fw) {
	if(logFor(LOG_FUN)) {
		log << "\tlevel = " << level << io::endl;
		log << "\tunrolling = " << unrolling << io::endl;
		log << "\tmust_entry = " << must_entry << io::endl;
		log << "\tpers_entry = " << pers_entry << io::endl;
	}

	FIRSTMISS_LEVEL(fw) = level;
	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iter cfg(INVOLVED_CFGS(fw)); cfg(); cfg++) {
		for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
			CACHE_ACS_MUST(*bb) = new Vector<MUSTProblem::Domain*>;
			if (level != FML_NONE)
				CACHE_ACS_PERS(*bb) = new Vector<PERSProblem::Domain*>;
		}
	}

	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION_FEATURE.get(fw)->instCache();

	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(fw, lbsets[i], cache);
	}
}

}
