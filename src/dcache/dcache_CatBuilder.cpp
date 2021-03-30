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
#include <otawa/cfg/Loop.h>
#include <otawa/dcache/CatBuilder.h>
#include <otawa/dcache/ACSMayBuilder.h>
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
CATBuilder::CATBuilder(p::declare& r)
:	Processor(r),
	firstmiss_level(DFML_NONE),
	wt_def_cat(cache::INVALID_CATEGORY),
	prob(nullptr),
	probMay(nullptr),
	has_pers(false)
{
}


/**
 */
void CATBuilder::processLBlock(otawa::Block *bb, BlockAccess& b, MUSTPERS::Domain& dom, MAYProblem::Domain& domMay, const Block *block) {
	ASSERT(bb != nullptr);
	ASSERT(block != nullptr);

	// initialization
	bool done = false;
	IN_ASSERT(bool alwaysHit = false);
	CATEGORY(b) = cache::NOT_CLASSIFIED;

	if(dom.getMust().contains(block->index())) {
		CATEGORY(b) = cache::ALWAYS_HIT;
		IN_ASSERT(alwaysHit = true);
	}

	else if(has_pers) { // persistent

		// find the initial header
		otawa::Block *header = 0;
		if (LOOP_HEADER(bb))
			header = bb;
	  	else
	  		header = ENCLOSING_LOOP_HEADER(bb);
		if (!header && bb->cfg()->callCount() == 1) { // temp solution
			CFG::CallerIter a = bb->cfg()->callers().begin();
			otawa::Block *cb = *a;
			if (LOOP_HEADER(cb))
				header = cb;
			else
				header = ENCLOSING_LOOP_HEADER(cb);
		}

		// look in the different levels
		for(int k = dom.getPers().length() - 1; k >= 0 && header; k--) {
			if(dom.getPers().isPersistent(block->index(), k)) {
				CATEGORY(b) = cache::FIRST_MISS;
				CATEGORY_HEADER(b) = header;
				done = true;
				break;
			}
			otawa::Block *nh = ENCLOSING_LOOP_HEADER(header);

			if (nh)
				header = nh;
			else if (header->cfg()->callCount() == 1) { // temp solution
				CFG::CallerIter a = header->cfg()->callers().begin();
				otawa::Block *cb = *a;
				if (LOOP_HEADER(cb))
					header = cb;
				else
					header = ENCLOSING_LOOP_HEADER(cb);
			}
		}
	} // end of else if(has_pers)

	// out of MAY ?
	if(!done && !domMay.contains(block->index())) {
		CATEGORY(b) = cache::ALWAYS_MISS;
		ASSERTP(alwaysHit == false, "AH has been set, this creates a conflict.")
	}
}


///
void CATBuilder::join(otawa::Block *v, BlockAccess& b, cache::category_t c1, otawa::Block *h1) {
	cache::category_t c2 = CATEGORY(b);
	if(c1 == c2) {
		if(c1 == cache::FIRST_MISS) {
			auto h2 = *CATEGORY_HEADER(b);
			if(h1 != h2) {
				if(h1->cfg() != h2->cfg())
					CATEGORY_HEADER(b) = Loop::of(v)->header();
				else if(!Loop::of(h1)->includes(Loop::of(h2)))
					CATEGORY_HEADER(b) = h1;
			}
		}
		return;
	}
	else if(c1 == cache::INVALID_CATEGORY)
		return;
	else if(c2 == cache::INVALID_CATEGORY) {
		CATEGORY(b) = c1;
		CATEGORY_HEADER(b) = h1;
	}
	else if(c1 == cache::ALWAYS_HIT && c2 == cache::FIRST_MISS)
		return;
	else if(c1 == cache::FIRST_MISS && c2 == cache::ALWAYS_HIT) {
		CATEGORY(b) = c1;
		CATEGORY_HEADER(b) = h1;
	}
	else {
		CATEGORY(b) = cache::NOT_CLASSIFIED;
		CATEGORY_HEADER(b).remove();
	}
}


/**
 */
void CATBuilder::processLBlockSet(WorkSpace *ws, const BlockCollection& coll, const hard::Cache *cache) {
	if(coll.count() == 0)
		return;

	// prepare problem
	int set = coll.cacheSet();
	prob = new MUSTPERS(&coll, ws, cache);
	MUSTPERS::Domain dom = prob->bottom();

	probMay = new MAYProblem(coll, ws, cache);
	MAYProblem::Domain domMay = probMay->entry();

	acs_stack_t empty_stack;
	if(logFor(LOG_FUN))
		log << "\tSET " << set << io::endl;

	// traverse all CFGs
	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++) {
		if(logFor(LOG_BB))
			log << "\t\tCFG " << cfgs->get(i) << io::endl;

		for(CFG::BlockIter bb = cfgs->get(i)->blocks(); bb(); bb++) {
			if(!bb->isBasic())
				continue;
			if(logFor(LOG_BB))
				log << "\t\t\t" << *bb << io::endl;

			// get the MUST ACS at block entry
			acs_table_t *ins = MUST_ACS(*bb); // get the entry ACS
			prob->setMust(dom, *ins->get(set)); // set the MUST domain

			// get the PERS ACS at block entry
			acs_table_t *pers = PERS_ACS(*bb);
			has_pers = pers;
			if(!has_pers)
				prob->emptyPers(dom);
			else {
				acs_stack_t *stack;
				acs_stack_table_t *stack_table = LEVEL_PERS_ACS(*bb);
				if(stack_table)
					stack = &stack_table->get(set);
				else
					stack = &empty_stack;
				prob->setPers(dom, *pers->get(set), *stack);
			}

			// get the MAY ACS at block entry
			if(MAY_ACS(*bb))
				domMay = *(MAY_ACS(*bb)->get(set));

			// traverse all accesses
			auto& bs = *DATA_BLOCKS(*bb);
			for(int i = 0; i < bs.count(); i++) {
				auto& b = bs[i];
				
				// special case: store + write-through => default category
				if(cache->writePolicy() == hard::Cache::WRITE_THROUGH && b.action() == dcache::BlockAccess::STORE)
					CATEGORY(b) = wt_def_cat;

				// other cases
				else
					switch(b.kind()) {
					case BlockAccess::ANY:
						CATEGORY(b) = cache::NOT_CLASSIFIED;
						break;
					case BlockAccess::BLOCK:
						if(b.block().set() == set)
							processLBlock(*bb, b, dom, domMay, &b.block());
						break;
					case BlockAccess::RANGE: {
							auto ab = b.blockIn(set);
							if(ab != nullptr) {
								if(!CATEGORY(b).exists())
									processLBlock(*bb, b, dom, domMay, ab);
								else {
									cache::category_t oc = *CATEGORY(b);
									otawa::Block *oh = *CATEGORY_HEADER(b);
									processLBlock(*bb, b, dom, domMay, ab);
									join(*bb, b, oc, oh);
								}
							}
							break;
						}
						break;
					}

				// update ACSs
				prob->update(dom, b);
				probMay->update(domMay, b);
			}
		}
	}

	// clean the problems
	delete prob;
	prob = nullptr;
	delete probMay;
	probMay = nullptr;
}


/**
 */
void CATBuilder::configure(const PropList &props) {
	Processor::configure(props);
	firstmiss_level = DATA_FIRSTMISS_LEVEL(props);
	wt_def_cat = dcache::WRITETHROUGH_DEFAULT_CAT(props);
}


/**
 */
void CATBuilder::processWorkSpace(otawa::WorkSpace *ws) {
	const BlockCollection *colls = DATA_BLOCK_COLLECTION(ws);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION_FEATURE.get(ws)->dataCache();

	// compute the categories
	for (int i = 0; i < cache->rowCount(); i++) {
		ASSERT(i == colls[i].cacheSet());
		processLBlockSet(ws, colls[i], cache );
	}

	// display the statistics
	displayStats(ws);
}


///
void CATBuilder::displayStats(WorkSpace *ws) {
	int stats[cache::TOP_CATEGORY];
	if(!logFor(LOG_FUN))
		return;

	// compute stats
	array::set(stats, cache::TOP_CATEGORY, 0);
	const CFGCollection *cfgs = INVOLVED_CFGS(ws);
	ASSERT(cfgs);
	for(int i = 0; i < cfgs->count(); i++) {
		for(CFG::BlockIter bb = cfgs->get(i)->blocks(); bb(); bb++) {
			if(logFor(LOG_INST))
				log << "\tBB " << bb->index() << " (" << bb->address() << ")\n";
			for(const auto& b: *DATA_BLOCKS(*bb)) {
				if(logFor(LOG_INST))
					log << "\t\t" << b << " -> " << CATEGORY(b) << io::endl;
				stats[0]++;
				stats[dcache::CATEGORY(b)]++;
			}
		}
	}

	if(logFor(LOG_FUN))
		if(stats[0] != 0) {
			for(int i = 1; i < cache::TOP_CATEGORY; i++)
				log << "\t" << cache::category_t(i)
					<< ": " << stats[i]
					<< " (" << io::fmt(stats[i] * 100. / stats[0]).width(6, 2)
					<< "%)\n";
			log << "\ttotal: " << stats[0] << io::endl;
		}

}



/**
 * This features ensures that a category each data block access have received
 * a category describing its hit/miss behavior.
 *
 * @par Configuration
 *	* @ref WRITETHROUGH_DEFAULT_CAT
 *
 * @par Default processor
 *	* @ref CATBuilder
 *
 * @par Properties
 *	* @ref CATEGORY
 *	*@ref CATEGORY_HEADER
 *
 * @ingroup dcache
 */
p::feature CATEGORY_FEATURE("otawa::dcache::CATEGORY_FEATURE", new Maker<CATBuilder>());


/**
 * Used to configure @ref CATEGORY_FEATURE.
 *
 * Specify the category of a store access when the cache supports write-through policy.
 * As a default, the category is @ref ALWAYS_HIT assuming that there is a Write Buffer to hide
 * the memory write time. If there is no Write Buffer, you can use this identifier
 * to configure category as @ref ALWAYS_MISS.
 */
p::id<cache::category_t> WRITETHROUGH_DEFAULT_CAT("otawa::dcache::WRITETHROUGH_DEFAULT_CAT", cache::ALWAYS_HIT);
// !!TODO!! This default value presumes that the architecture uses also a write buffer:
// it is not always true and the right value should be INVALID and a WriteBuffer description
// should be added to the cache description.


/**
 * When a cache access has a category of @ref cache::FIRST_MISS, the "first" part
 * is relative to a particular loop whose header is given by this property.
 *
 * @p Hook
 * @li @ref BlockAccess
 * @ingroup dcache
 */
p::id<otawa::Block *> CATEGORY_HEADER("otawa::dcache::CATEGORY_HEADER", 0);


/**
 * Gives the category for a data cache block access.
 *
 * @p Hook
 * @li @ref BlockAccess
 * @ingroup dcache
 */
p::id<cache::category_t> CATEGORY("otawa::dcache::CATEGORY", cache::NOT_CLASSIFIED); // changed from INVALID_CATEGORY, which can be used to check the soundness


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
