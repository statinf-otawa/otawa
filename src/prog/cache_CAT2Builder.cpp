/*
 *	$Id $
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

/**
 *
 * In the case of a FIRST_HIT (or FIRST_MISS) property, contains the header
 * of the loop causing the HIT (or MISS) at the first iteration.
 *
 * @par Hooks
 * @li @ref LBlocks
 */
Identifier<BasicBlock*> CATEGORY_HEADER("otawa::CATEGORY_HEADER", 0);


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

DEFINE_PROC(otawa::CAT2Builder,
	version(1, 0, 0);
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_ACS_FEATURE);
	require(ICACHE_FIRSTLAST_FEATURE);
	provide(ICACHE_CATEGORY2_FEATURE);
)


/**
 * !!TODO!!
 */
void CAT2Builder::processLBlockSet(otawa::CFG *cfg, LBlockSet *lbset, const hard::Cache *cache) {
	int line = lbset->line();

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
				CATEGORY(lblock) = NOT_CLASSIFIED;
			} else {
				CATEGORY(lblock) = ALWAYS_MISS;
			}

			if (must->contains(lblock->cacheblock())) {
				CATEGORY(lblock) = ALWAYS_HIT;
			} else if (may && !may->contains(lblock->cacheblock())) {
				CATEGORY(lblock) = ALWAYS_MISS;
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

		// record stats
		if(cstats)
			cstats->add(CATEGORY(lblock));
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
	cstats = CATEGORY_STATS(props);
	if(cstats)
		cstats->reset();
}


/**
 */
void CAT2Builder::processCFG(otawa::WorkSpace *fw, otawa::CFG *cfg) {
	//int i;
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();

	for (int i = 0; i < cache->rowCount(); i++) {
		processLBlockSet(cfg, lbsets[i], cache );
	}
}


/**
 * !!TODO!!
 */
Feature<CAT2Builder> ICACHE_CATEGORY2_FEATURE("otawa::ICACHE_CATEGORY2_FEATURE");


/**
 * @class CategoryStats
 * This class is used to store statistics about the categories about cache
 * accesses. It it provided by cache category builders.
 * @see CATBuilder, CAT2Builder
 */

/**
 */
CategoryStats::CategoryStats(void) {
	reset();
}

/**
 * Reset the statistics.
 */
void CategoryStats::reset(void) {
	_total = 0;
	_linked = 0;
	for(int i = 0; i <= NOT_CLASSIFIED; i++)
		counts[i] = 0;
}

/**
 * @fn  void Categorystats::add(category_t cat);
 * Increment the counter for the given category.
 * @param cat	Category to increment the counter.
 */

/**
 * @fn void CategoryStats::addLinked(void);
 * Add a new linked l-block to the statistics.
 */

/**
 * @fn int CategoryStats::get(category_t cat) const;
 * Get the counter of a category.
 * @param cat	Category to get counter for.
 * @return		Category count.
 */

/**
 * @fn int CategoryStats::total(void) const;
 * Get the total count of categories.
 * @return		Category total count.
 */

/**
 * @fn int CategoryStats::linked(void) const;
 * Get the count of linked statistics.
 * @return	Linked l-block statistics.
 */


/**
 * Put in the statistics to get statistics about cache categories.
 *
 * @par Hooks
 * @li processor configuration property list
 */
Identifier<CategoryStats *> CATEGORY_STATS("otawa::CATEGORY_STATS", 0);


/**
 */
io::Output& operator<<(io::Output& out, const CategoryStats& stats) {
	static cstring names[] = {
			"invalid",
			"always-hit",
			"first-hit",
			"first-miss",
			"always-miss",
			"not-classified"
	};

	for(int i = ALWAYS_HIT; i <= NOT_CLASSIFIED; i++)
		out << names[i] << '\t' << (float(stats.get(category_t(i))) * 100 / stats.total())
			<< "% (" << stats.get(category_t(i)) << ")\n";
	out << "total\t\t100% (" << stats.total() << ")\n";
	out << "linked\t\t" << (float(stats.linked()) * 100 / stats.total())
		<< "% (" << stats.linked() << ")\n";
	return out;
}

}
