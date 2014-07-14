/*
 *	dcache::ACSMayBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/hard/Platform.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/dcache/CatBuilder.h>
#include <otawa/dcache/ACSMayBuilder.h>
#include <otawa/cache/categories.h>
#include <otawa/dcache/features.h>
#include <otawa/dcache/MUSTPERS.h>

namespace otawa { namespace dcache {

/**
 * @class CATBuilder
 * Processor to get the categories for each block access of a data cache.
 *
 * @p Provided features
 * @li @ref CATEGORY_FEATURE
 *
 * @p Required features
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref MUST_ACS_FEATURE
 *
 * @p Configuration
 * @li @ref DATA_FIRSTMISS_LEVEL
 * @ingroup dcache
 */

p::declare CATBuilder::reg = p::init("otawa::dcache::CATBuilder", Version(1, 0, 0))
	.maker<CATBuilder>()
	.base(Processor::reg)
	.require(DOMINANCE_FEATURE)
	.require(LOOP_HEADERS_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(MUST_ACS_FEATURE)
	.provide(CATEGORY_FEATURE);


/**
 */
CATBuilder::CATBuilder(p::declare& r): Processor(r), firstmiss_level(DFML_NONE) {
}


/**
 */
void CATBuilder::cleanup(WorkSpace *ws) {
	static cstring cat_names[] = {
			"INV",
			"AH",
			"FH",
			"FM",
			"AM",
			"NC"
	};

	if(!logFor(LOG_BB))
		return;

	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++)
		for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++) {
			cerr << "\tBB " << bb->number() << " (" << bb->address() << ")\n";
			Pair<int, BlockAccess *> ab = DATA_BLOCKS(bb);
			for(int j = 0; j < ab.fst; j++) {
				BlockAccess& b = ab.snd[j];
				cerr << "\t\t" << b << " -> " << cat_names[dcache::CATEGORY(b)] << io::endl;
			}
		}
}


/**
 */
void CATBuilder::processLBlockSet(WorkSpace *ws, const BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;

	// prepare problem
	int line = coll.cacheSet();
	MUSTPERS prob(&coll, ws, cache);
	MUSTPERS::Domain dom = prob.bottom();
	acs_stack_t empty_stack;
	if(logFor(LOG_FUN))
		log << "\tSET " << line << io::endl;

	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++) {
		if(logFor(LOG_BB))
			log << "\t\tCFG " << cfgs->get(i) << io::endl;

		for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++) {
			if(logFor(LOG_BB))
				log << "\t\t\t" << *bb << io::endl;

			// get the input domain
			acs_table_t *ins = MUST_ACS(bb);
			prob.setMust(dom, *ins->get(line));
			acs_table_t *pers = PERS_ACS(bb);
			bool has_pers = pers;
			if(!has_pers)
				prob.emptyPers(dom);
			else {
				acs_stack_t *stack;
				acs_stack_table_t *stack_table = LEVEL_PERS_ACS(bb);
				if(stack_table)
					stack = &stack_table->item(line);
				else
					stack = &empty_stack;
				prob.setPers(dom, *pers->get(line), *stack);
			}

			// explore the adresses
			Pair<int, BlockAccess *> ab = DATA_BLOCKS(bb);
			for(int j = 0; j < ab.fst; j++) {
				BlockAccess& b = ab.snd[j];
				if(b.kind() != BlockAccess::BLOCK) {
					CATEGORY(b) = cache::NOT_CLASSIFIED;
					prob.ageAll(dom);
				}
				else if(b.block().set() == line) {

					// initialization
					bool done = false;
					CATEGORY(b) = cache::NOT_CLASSIFIED;
					ACS *may = 0;
					if(MAY_ACS(bb) != 0)
						may = MAY_ACS(bb)->get(line);

					// in MUST ?
					if(dom.getMust().contains(b.block().index()))
						CATEGORY(b) = cache::ALWAYS_HIT;

					// persistent ?
					else if(has_pers) {

						// find the initial header
						BasicBlock *header;
						if (LOOP_HEADER(bb))
							header = bb;
					  	else
					  		header = ENCLOSING_LOOP_HEADER(bb);

						// look in the different levels
						for(int k = dom.getPers().length() - 1; k >= 0 && header; k--) {
							if(dom.getPers().isPersistent(b.block().index(), k)) {
								CATEGORY(b) = FIRST_MISS;
								CATEGORY_HEADER(b) = header;
								done = true;
								break;
							}
							header = ENCLOSING_LOOP_HEADER(header);
						}
					}

					// out of MAY ?
					if(!done && may && !may->contains(b.block().index()))
						CATEGORY(b) = cache::ALWAYS_MISS;

					// update state
					prob.inject(dom, b.block().index());
				}
			}
		}
	}
}


/**
 */
void CATBuilder::configure(const PropList &props) {
	Processor::configure(props);
	firstmiss_level = DATA_FIRSTMISS_LEVEL(props);
	//cstats = CATEGORY_STATS(props);
	//if(cstats)
	//	cstats->reset();
}


/**
 */
void CATBuilder::processWorkSpace(otawa::WorkSpace *fw) {
	//int i;
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->dataCache();

	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].cacheSet());
		processLBlockSet(fw, colls[i], cache );
	}
}

static SilentFeature::Maker<CATBuilder> maker;
/**
 * This features ensures that a category each data block access have received
 * a category describing its hit/miss behavior.
 *
 * @p Default processor
 * @li @ref CATBuilder
 *
 * @p Properties
 * @li @ref CATEGORY
 * @li @ref CATEGORY_HEADER
 * @ingroup dcache
 */
SilentFeature CATEGORY_FEATURE("otawa::dcache::CATEGORY_FEATURE", maker);


/**
 * When a cache access has a category of @ref cache::FIRST_MISS, the "first" part
 * is relative to a particular loop whose header is given by this property.
 *
 * @p Hook
 * @li @ref BlockAccess
 * @ingroup dcache
 */
Identifier<BasicBlock*> CATEGORY_HEADER("otawa::dcache::CATEGORY_HEADER", 0);


/**
 * Gives the category for a data cache block access.
 *
 * @p Hook
 * @li @ref BlockAccess
 * @ingroup dcache
 */
Identifier<cache::category_t> CATEGORY("otawa::dcache::category", cache::INVALID_CATEGORY);


/**
 * @class CategoryStats
 * This class is used to store statistics about the categories about cache
 * accesses. It it provided by cache category builders.
 * @see CATBuilder, CAT2Builder
 * @ingroup dcache
 */

/**
 */
/*CategoryStats::CategoryStats(void) {
	reset();
}*/

/**
 * Reset the statistics.
 */
/*void CategoryStats::reset(void) {
	_total = 0;
	_linked = 0;
	for(int i = 0; i <= NOT_CLASSIFIED; i++)
		counts[i] = 0;
}*/

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
//Identifier<CategoryStats *> CATEGORY_STATS("otawa::CATEGORY_STATS", 0);


/**
 */
/*io::Output& operator<<(io::Output& out, const CategoryStats& stats) {
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
}*/

} }	// otawa::dcache
