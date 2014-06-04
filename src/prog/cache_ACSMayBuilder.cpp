/*
 *	ACSMayBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
using namespace elm;




namespace otawa {

/**
 *
 * This feature represents the availability of MAY Abstract Cache State informations.
 *
 * @par Properties
 * @li @ref CACHE_ACS
 *
 * @par Processors
 * @li @ref ACSMayBuilder
 */
p::feature ICACHE_ACS_MAY_FEATURE("otawa::ICACHE_ACS_MAY_FEATURE", new Maker<ACSMayBuilder>());

/**
 * This property represents the "may" Abstract Cache State of a basic block.
 * The vector stores the abstract cache states corresponding to all cache lines.
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
 Identifier<genstruct::Vector<MAYProblem::Domain*>* > CACHE_ACS_MAY("otawa::CACHE_ACS_MAY", NULL);

/**
 * This property allows us to set an entry may ACS.
 *
 * @par Hooks
 * @li @ref PropList
 */
 Identifier<Vector<MAYProblem::Domain*>* > CACHE_ACS_MAY_ENTRY("otawa::CACHE_ACS_MAY_ENTRY", NULL);


/**
 * @class ACSMayBuilder
 *
 * This processor produces the Abstract Cache States (ACS), for the MAY ACS.
 * The MAY ACS represents cache blocks which may be in the cache. It is only useful for differentiating between
 * NOT_CLASSIFIED and ALWAYS_MISS.
 *
 * @par Configuration
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

p::declare ACSMayBuilder::reg = p::init("otawa::ACSMayBuilder", Version(1, 0, 0))
	.maker<ACSMayBuilder>()
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.require(ICACHE_FIRSTLAST_FEATURE)
	.provide(ICACHE_ACS_MAY_FEATURE);


ACSMayBuilder::ACSMayBuilder(p::declare& r): Processor(r) {
}


void ACSMayBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset, const hard::Cache *cache) {
	int line = lbset->line();
	if(logFor(LOG_CFG) && lbset->count() > 2)
		log << "\tSET " << line << io::endl;
	MAYProblem mayProb(lbset->cacheBlockCount(), lbset, fw, cache, cache->wayCount());

	// analysis with unrolling
	if (unrolling) {
		UnrollingListener<MAYProblem> mayList(fw, mayProb);
		FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > mayFp(mayList);
		HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<MAYProblem> > > mayHai(mayFp, *fw);
		mayHai.solve(NULL, may_entry ? may_entry->get(line) : NULL);
		for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++)
			for (CFG::BBIterator bb(cfg); bb; bb++)
				CACHE_ACS_MAY(bb)->add(new MAYProblem::Domain(*mayList.results[cfg->number()][bb->number()]));
	}

	// analysis without unrolling
	else {
		DefaultListener<MAYProblem> mayList(fw, mayProb);
		DefaultFixPoint<DefaultListener<MAYProblem> > mayFp(mayList);
		HalfAbsInt<DefaultFixPoint<DefaultListener<MAYProblem> > > mayHai(mayFp, *fw);
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

	// Build the vectors for receiving the ACS...
	for (CFGCollection::Iterator cfg(INVOLVED_CFGS(fw)); cfg; cfg++) {
		for (CFG::BBIterator bb(cfg); bb; bb++)
			CACHE_ACS_MAY(bb) = new genstruct::Vector<MAYProblem::Domain*>;
	}

	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();

	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(fw, lbsets[i], cache);
	}
}

}
