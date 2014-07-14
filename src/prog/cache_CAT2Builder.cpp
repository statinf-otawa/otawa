/*
 *	CAT2Builder processor implementation
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

using namespace cache;

/**
 */
CAT2Builder::CAT2Builder(AbstractRegistration& registration): CFGProcessor(registration) {
}

Registration<CAT2Builder> CAT2Builder::reg(
	"otawa::CAT2Builder",
	Version(1, 0, 0),
	p::require, &DOMINANCE_FEATURE,
	p::require, &LOOP_HEADERS_FEATURE,
	p::require, &LOOP_INFO_FEATURE,
	p::require, &COLLECTED_LBLOCKS_FEATURE,
	p::require, &ICACHE_ACS_FEATURE,
	p::require, &ICACHE_FIRSTLAST_FEATURE,
	p::provide, &ICACHE_CATEGORY2_FEATURE,
	p::end
);


/**
 * @class CAT2Builder
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

/**
 * !!TODO!!
 */
void CAT2Builder::processLBlockSet(otawa::CFG *cfg, LBlockSet *lbset, const hard::Cache *cache) {
	int line = lbset->line();
	if(logFor(LOG_CFG) && lbset->count() > 2)
		log << "\tSET " << lbset->line() << io::endl;

	// Use the results to set the categorization
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
				cache::CATEGORY(lblock) = cache::NOT_CLASSIFIED;
			} else {
				cache::CATEGORY(lblock) = cache::ALWAYS_MISS;
			}

			if (must->contains(lblock->cacheblock())) {
				cache::CATEGORY(lblock) = cache::ALWAYS_HIT;
			} else if (may && !may->contains(lblock->cacheblock())) {
				cache::CATEGORY(lblock) = cache::ALWAYS_MISS;
			} else if (firstmiss_level != FML_NONE) {
				if (LOOP_HEADER(lblock->bb()))
					header = lblock->bb();
			  	else header = ENCLOSING_LOOP_HEADER(lblock->bb());

			  	int bound;
			  	bool perfect_firstmiss = true;
				PERSProblem::Domain *pers = CACHE_ACS_PERS(lblock->bb())->get(line);
				bound = 0;

				if ((pers->length() > 1) && (firstmiss_level == FML_INNER))
					bound = pers->length() - 1;
				cache::CATEGORY_HEADER(lblock) = NULL;
			  	for (int k = pers->length() - 1 ; (k >= bound) && (header != NULL); k--) {
					if (pers->isPersistent(lblock->cacheblock(), k)) {
						cache::CATEGORY(lblock) = cache::FIRST_MISS;
						cache::CATEGORY_HEADER(lblock) = header;
					} else perfect_firstmiss = false;
					header = ENCLOSING_LOOP_HEADER(header);
				}

				if ((firstmiss_level == FML_OUTER) && (perfect_firstmiss == false))
					cache::CATEGORY(lblock) = cache::ALWAYS_MISS;
			} /* of category condition test */
		} else
			cache::CATEGORY(lblock) = cache::ALWAYS_MISS;

		// record stats
		total_cnt++;
		switch(cache::CATEGORY(lblock)) {
		case cache::ALWAYS_HIT:		ah_cnt++; break;
		case cache::ALWAYS_MISS:	am_cnt++; break;
		case cache::FIRST_MISS:		pers_cnt++; break;
		case cache::NOT_CLASSIFIED:	nc_cnt++; break;
		}
		if(logFor(LOG_BB)) {
			log << "\t\t" << lblock->address() << ": " << *cache::CATEGORY(lblock);
			if(cache::CATEGORY_HEADER(lblock))
				log << " (" << *cache::CATEGORY_HEADER(lblock) << ")";
			log << io::endl;
		}
		if(cstats)
			cstats->add(cache::CATEGORY(lblock));
	}


}


/**
 * !!TODO!!
 */
void CAT2Builder::setup(WorkSpace *fw) {
}


/**
 */
void CAT2Builder::configure(const PropList &props) {
	CFGProcessor::configure(props);
	firstmiss_level = FIRSTMISS_LEVEL(props);
	cstats = cache::CATEGORY_STATS(props);
	if(cstats)
		cstats->reset();
}


/**
 */
void CAT2Builder::processCFG(otawa::WorkSpace *fw, otawa::CFG *cfg) {
	//int i;
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();

	total_cnt = 0;
	ah_cnt = 0;
	am_cnt = 0,
	pers_cnt = 0;
	nc_cnt = 0;

	for (int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(cfg, lbsets[i], cache );

	if(logFor(LOG_PROC)) {
		log << "\ttotal = " << total_cnt << io::endl;
		if(total_cnt)
			log << "\tAH = " << ah_cnt << " (" << (ah_cnt * 100 / total_cnt) << "%)\n"
				<< "\tAM = " << am_cnt << " (" << (am_cnt * 100 / total_cnt) << "%)\n"
				<< "\tPERS = " << pers_cnt << " (" << (pers_cnt * 100 / total_cnt) << "%)\n"
				<< "\tAM = " << nc_cnt << " (" << (nc_cnt * 100 / total_cnt) << "%)\n";
	}
}


static SilentFeature::Maker<CAT2Builder> cat_maker;
/**
 * This feature ensures that the categories for instruction cache according to the following
 * method has been computed:
 *
 * C. Ballabriga, H. Cassé. Improving the First-Miss Computation in Set-Associative Instruction Caches.
 * Euromicro Conference on Real-Time Systems (ECRTS 2008), IEEE, p. 341-350, 2008.
 *
 * @par Properties
 * @li @ref CATEGORY
 * @li @ref  CATEGORY_HEADER
 *
 * @par Processors
 * @li @ref CAT2Builder (default)
 */
SilentFeature ICACHE_CATEGORY2_FEATURE("otawa::ICACHE_CATEGORY2_FEATURE", cat_maker);

}	// otawa
