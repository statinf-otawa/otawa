/*
 *	ipet::CacheSupport implementation
 *	Copyright (c) 2015, IRIT UPS <casse@irit.fr>
 *
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/cache/cat2/features.h>
#include <otawa/cfg.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/dcache/features.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ipet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/TrivialDataCacheManager.h>
#include <otawa/ipet/TrivialInstCacheManager.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/DynFeature.h>
#include <otawa/proc/Feature.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa { namespace ipet {

static Registration<TrivialInstCacheManager> my_reg(
	"otawa::ipet::TrivialInstCacheManager",
	Version(1, 1, 0),
	p::provide, &INST_CACHE_SUPPORT_FEATURE,
	p::require, &BB_TIME_FEATURE,
	p::require, &hard::CACHE_CONFIGURATION_FEATURE,
	p::end
);

/**
 * @class TrivialInstCacheManager
 * This processor is a trivial manager of instruction cache for IPET approach.
 * Basically, it considers that each code line blocks causes a miss.
 *
 * @par Provided Features
 * @li @ref ipet::INST_CACHE_SUPPORT_FEATURE
 *
 * @par Required Features
 * @li @ref ipet::BB_TIME_FEATURE
 */


/**
 */
TrivialInstCacheManager::TrivialInstCacheManager(void): BBProcessor(my_reg), cache(0) {
}


/**
 */
void TrivialInstCacheManager::setup(WorkSpace *fw) {
	const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION(fw);
	cache = conf->instCache();
	if(!cache)
		warn("no instruction cache available.");
}


/**
 */
void TrivialInstCacheManager::cleanup(WorkSpace *fw) {
	cache = 0;
}


/**
 */
void TrivialInstCacheManager::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb)
{
	if(cache) {
		int misses = 0;
		int size = bb->size();
		int offset = cache->offset(bb->address());
		if(offset) {
			misses++;
			size -= offset;
		}
		misses += size >> cache->blockBits();
		TIME(bb) = TIME(bb) + misses * cache->missPenalty();
	}
}

/**
 * This feature ensurers that the instruction cache has been modelled
 * in the IPET approach.
 */
p::feature INST_CACHE_SUPPORT_FEATURE("otawa::ipet::INST_CACHE_SUPPORT_FEATURE", new Maker<TrivialInstCacheManager>);


/**
 * @class TrivialDataCacheManager
 * This processor apply a simple method for managing data cache in IPET method.
 * It adds to the basic block execution time the sum of penalties induced by
 * each memory access instruction in the basic block.
 */

void TrivialDataCacheManager::configure(WorkSpace *framework) {
	fw = framework;
	if(!hard::CACHE_CONFIGURATION(fw)->hasDataCache()) {
		time = 0;
		log << "WARNING: there is no data cache here !\n";
	}
	else
		time = fw->cache().dataCache()->missPenalty();
}


/**
 * Build the trivial data cache manager.
 */
TrivialDataCacheManager::TrivialDataCacheManager(void)
: BBProcessor("ipet::TrivialDataCacheManager", Version(1, 0, 0)), fw(0), time(0) {
	provide(DATA_CACHE_SUPPORT_FEATURE);
	require(COLLECTED_CFG_FEATURE);
	require(BB_TIME_FEATURE);
}


/**
 */
void TrivialDataCacheManager::processBB(WorkSpace *framework, CFG *cfg,
BasicBlock *bb) {
	ASSERT(framework);
	ASSERT(cfg);
	ASSERT(bb);

	// Check configuration
	if(framework != fw)
		configure(framework);
	if(!time)
		return;

	// Do not process entry and exit
	if(bb->isEntry() || bb->isExit())
		return;

	// Count the memory accesses
	int count = 0;
	for(BasicBlock::InstIter inst(bb); inst; inst++)
		if(inst->isMem())
			count++;

	// Store new BB time
	TIME(bb) = TIME(bb) + count * time;
}


/**
 * This feature ensures that the first-level data cache has been taken in
 * account in the basic block timing.
 */
p::feature DATA_CACHE_SUPPORT_FEATURE("otawa::ipet::DATA_CACHE_SUPPORT_FEATURE", new Maker<TrivialDataCacheManager>());


/**
 * Provide support for cache hierarchy. Basically, explore the cache hierarchy
 * and either select the matching analyzes, or raise an error.
 */
class CacheSupport: public Processor {
public:
	static p::declare reg;
	CacheSupport(p::declare& _reg = reg): Processor(_reg) { }

protected:
	virtual void prepare(WorkSpace *ws) {
		const hard::CacheConfiguration& conf = **hard::CACHE_CONFIGURATION(ws);

		// no cache needed?
		if(!conf.instCache() && !conf.dataCache()) {
			if(logFor(LOG_PROC))
				log << "\tINFO: no cache.";
			return;
		}

		// unified?
		if(conf.isUnified())
			throw ProcessorException(*this, "unified L1 cache unsupported");

		// process instruction cache
		if(conf.instCache()) {
			if(conf.instCache()->replacementPolicy() != hard::Cache::LRU)
				throw ProcessorException(*this, _ << "instruction cache L1 unsupported");
			require(ICACHE_ACS_FEATURE);
			require(ICACHE_ACS_MAY_FEATURE);
			require(ICACHE_CATEGORY2_FEATURE);
			require(ICACHE_ONLY_CONSTRAINT2_FEATURE);
		}

		// process data cache
		if(conf.dataCache()) {
			if(conf.dataCache()->replacementPolicy() != hard::Cache::LRU)
				throw ProcessorException(*this, _ << "replacement policy of data cache L1 unsupported");
			// TODO		For now, supports only write-through data cache (write-back to be added later)
			if(conf.dataCache()->writePolicy() != hard::Cache::WRITE_THROUGH)
				throw ProcessorException(*this, _ << "write policy of data cache L1 unsupported");
			requireDyn("otawa::dcache::MUST_ACS_FEATURE");
			requireDyn("otawa::dcache::PERS_ACS_FEATURE");
			requireDyn("otawa::dcache::MAY_ACS_FEATURE");
			requireDyn("otawa::dcache::CONSTRAINTS_FEATURE");
		}
	}

private:
	void requireDyn(cstring name) {
		DynFeature f(name);
		require(f);
	}
};

p::declare CacheSupport::reg = p::init("otawa::ipet::CacheSupport", Version(1, 0, 0))
	.maker<CacheSupport>()
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.provide(CACHE_SUPPORT_FEATURE);


/**
 * This feature ensures that analysis for the cache configuration has been performed.
 */
p::feature CACHE_SUPPORT_FEATURE("otawa::ipet::CACHE_SUPPORT_FEATURE", new Maker<CacheSupport>());

} } // otawa::ipet
