/*
 *	dcache plugin hook
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/dcache/features.h>

using namespace elm;
using namespace otawa;

namespace otawa { namespace dcache {

/**
 * @class BlockAccess
 * This class represents a data cache access that is composed by;
 *	* the instruction that performs it,
 *	* the performed action (BlockAccess::LOAD or BlockAccess::STORE),
 *	* the accessed memory.
 *
 * The accessed memory may BlockAccess::ANY if any part of the memory can be
 * accessed (often because of lack of precision of the analysis).
 * BlockAccess::BLOCK if a single block is accessed. BlockAccess::RANGE if
 * a range of memory is accessed which size (in block count) is less than the
 * way size.
 * @ingroup dcache
 */

/**
 * Test if the given set concerns the range access.
 * @param set	Set to test for.
 * @param cache	Current cache (to get set information).
 * @return		True if the set contains a block of the range, false else.
 */
bool BlockAccess::inSet(int set, const hard::Cache *cache) const {

	// range over the whole cache
	if(first() < last()) {
		if(int(cache->tag(last()) - cache->tag(first()))  >= cache->setCount())
			return true;
	}
	else if(int(cache->tag(first()) + cache->setCount() - cache->tag(last())) >= cache->setCount())
		return true;

	// computes sets
	if(cache->set(first()) < cache->set(last()))
		return int(cache->set(first())) <= set && set <= int(cache->set(last()));
	else
		return set <= int(cache->set(first())) || int(cache->set(last())) <= set;
}


/**
 * Test if the given block may be concerned by the current access.
 * @param block		Block to test.
 * @return			True if it concerned, false else.
 */
bool BlockAccess::in(const Block& block) const {
	switch(kind()) {
	case ANY:
		return true;
	case BLOCK:
		return data.blk->index() == block.index();
	case RANGE:
		if(first() <= last())
			return Address(first()) <= block.address() && block.address() <= Address(last());
		else
			return block.address() <= Address(first()) || Address(last()) <= block.address();
	default:
		ASSERT(false);
		return false;
	}
}

/**
 * @fn const Vector<const Block*>& getBlocks(void) const;
 * Get the list of accessed blocks.
 * @warning This function is only valid for a RANGE access.
 * @return	Accessed blocks.
 */

/**
 * Get the block corresponding to the given set.
 * @warning This function is only valid for a RANGE access.
 * @return	Corresponding block or null pointer.
 */
const Block *BlockAccess::blockIn(int set) const {
	switch(_kind) {
	case BLOCK:
		if(data.blk->set() == set)
			return data.blk;
		break;
	case RANGE:
		if(!inRange(set))
			return nullptr;
		else if(first() <= last() || set >= first())
			return blocks()[set - first()];
		else
			return blocks()[set + (data.range->setc - first())];
	}
	return nullptr;
}

/**
 * @defgroup dcache Data Cache
 *
 * This module is dedicated to the categorisation of data cache accesses.
 * As for the instruction cache, four categories are handled:
 *	* @ref	otawa::cache::ALWAYS_HIT if the access results always in a hit,
 *	* @ref	otawa::cache::FIRST_MISS (also named persistent) if the first access
 *		is unknown and the following accesses results in hits,
 *	* @ref otawa::cache::ALWAYS_MISS if the access results always in a miss,
 *	* @ref otawa::cache::NOT_CLASSIFIED if the previous categories do not apply.
 *
 * This module supports the following data cache configuration:
 *	* replacement policy -- LRU
 *	* write policy -- write-through, write-back (with dirty+purge analysis).
 *
 * The data cache description is obtained from the @ref otawa::hard::CACHE_CONFIGURATION_FEATURE
 * feature and the cache addresses are obtained from @ref otawa::ADDRESS_ANALYSIS_FEATURE.
 * In OTAWA, there are different feature to obtain the addresses represented by
 * the following features:
 *	* otawa::dcache::CLP_BLOCK_FEATURE -- use the plug-in CLP for address representation.
 *
 * To select which address provider to use, one has to requireone of the previous
 * by hand before running other data cache analyses.
 *
 * The different phases to perform data cache analyses are:
 *	* obtain data cache blocks with one data block provider (listed above) --
 *		the result is a list of block accesses (@ref otawa::dcache::BlockAccess)
 * 		hooked to basic blocks with dcache::DATA_BLOCKS properties,
 *	* ACS computation -- according to the accesses list, the ACS (Abstract Cache
 *		State) are computed for each mode MUST, PERS and/or MAY analysis
 *		(dcache::MUST_ACS_FEATURE, dcache::PERS_ACS_FEATURE, dcache::MAY_ACS_FEATURE),
 *	* category derivation -- from the ACS computed in the previous phases,
 *		a category is computed and linked to each block access (dcache::CATEGORY_FEATURE),
 *	* constraint generation in ILP system to bound the variable count the misses
 *		(dcache::CONSTRAINTS_FEATURE),
 *	* time computation -- from the categories, the execution time of a block may
 *		be computed and this feature provides a very trivial way to include
 *		this time in the objective function of ILP system (dcache::WCET_FUNCTION_FEATURE),
 *	* dirtiness and purge analysis is only required for write-back data caches --
 *	  it analyze the dirty bit of cache blocks and depending on their value
 *	  derives if a cache block may/must be written back to memory at replacement
 *	  time (dcache::DIRTY_FEATURE, dcache::PURGE_FEATURE).
 *
 * dcache::WCET_FUNCTION_FEATURE naively add the miss time to the block time.
 * An alternate and more precise approach is to use @ref etime Execution Graph
 * to embed the misses as event in the pipeline execution time calculation.
 *
 * Notice that the MAY is only optional and must be called by hand. In the same way, there is no persistence
 * analysis unless the persistence level is passed at configuration.
 *
 * To use this module, pass it name to the @c otawa-config utility: otawa-config dcache.
 */

class Plugin: public ProcessorPlugin {
public:
	Plugin(void): ProcessorPlugin("otawa::dcache", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
};

} } // otawa::dcache

otawa::dcache::Plugin otawa_dcache;
ELM_PLUGIN(otawa_dcache, OTAWA_PROC_HOOK);

