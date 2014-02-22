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
 * Test if the given set concerns the range access.
 * @param set	Set to test for.
 * @param cache	Current cache (to get set information).
 * @return		True if the set contains a block of the range, false else.
 */
bool BlockAccess::inSet(int set, const hard::Cache *cache) const {

	// range over the whole cache
	if(first() < last()) {
		if(cache->tag(last()) - cache->tag(first())  >= cache->setCount())
			return true;
	}
	else if(cache->tag(first()) + cache->setCount() - cache->tag(last()) >= cache->setCount())
		return true;

	// computes sets
	if(cache->set(first()) < cache->set(last()))
		return cache->set(first()) <= set && set <= cache->set(last());
	else
		return set <= cache->set(first()) || cache->set(last()) <= set;
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
	}
}


/**
 * @defgroup dcache Data Cache
 *
 * This module is dedicated to the categorisation of data cache accesses.
 * As for the instruction cache, four categories are handled:
 * @li @ref	otawa::cache::ALWAYS_HIT if the access results always in a hit,
 * @li @ref	otawa::cache::FIRST_MISS (also named persistent) if the first access is unknown and the following accesses results in hits,
 * @li @ref otawa::cache::ALWAYS_MISS if the access results always in a miss,
 * @li @ref otawa::cache::NOT_CLASSIFIED if the previous categories do not apply.
 *
 * The data cache description is obtained from the @ref otawa::hard::CACHE_CONFIGURATION_FEATURE feature
 * and the cache references are obtained from @ref otawa::ADDRESS_ANALYSIS_FEATURE. For the time being, this feature
 * is loosely implemented by @ref otawa::StackAnalysis. As this module may accepts several different address provider,
 * this processor must be runned by hand before the invocation of the processors of this module.
 *
 * Basically, the following phases are performed:
 * @li computation of block accesses -- for each basic block and instruction performing a cache access,
 * a list of accesses (@ref otawa::dcache::BlockAccess) is built and stored in the basic block,
 * @li ACS computation -- according to the accesses list, the ACS (Abstract Cache State) are computed (MUST, persistence and/or MAY analysis),
 * @li category derivation -- from the ACS computed in the previous phases, a category is computed and linked
 * to each block access,
 * @li time computation -- from the categories, the execution time of a block may be computed; yet, although this module
 * very trivial way to include this time (@ref otawa::dcache::CONSTRAINTS_FEATURE), more precise and sound results
 * are obtained by other integrated block time computation methods like ExeGraph.
 *
 * Notice that the MAY is only optional and must be called by hand. In the same way, there is no persistence
 * analysis unless the persistence level is passed at configuration.
 *
 * A category computation for data cache may look like:
 * @code
 * #include <otawa/util/StackAnalysis.h>
 * #include <otawa/dcache/features.h>
 *
 * using namespace otawa;
 *
 * void process(WorkSpace *ws) {
 * 	PropList props;
 * 	ws->require(STACK_ANALYSIS_FEATURE, props);
 * 	if(i_want_may_analysis)
 * 		ws->require(dcache::MAY_ACS_FEATURE);
 * 	if(i_want_persistence_analysis)
 *		dcache::DATA_FIRSTMISS_LEVEL(props) = dcache::DFML_MULTI;
 *	ws->require(dcache::CATEGORY_FEATURE, props);
 * }
 * @endcode
 *
 * To use this module, pass it name to the @c otawa-config utility: otawa-config --libs dcache.
 */

class Plugin: public ProcessorPlugin {
public:
	typedef genstruct::Table<AbstractRegistration * > procs_t;

	Plugin(void): ProcessorPlugin("otawa::dcache", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
	virtual procs_t& processors(void) const { return procs_t::EMPTY; };
};

} } // otawa::dcache

otawa::dcache::Plugin OTAWA_PROC_HOOK;
otawa::dcache::Plugin& otawa_dcache = OTAWA_PROC_HOOK;
