/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	TrivialInstCacheManager class implementation
 */
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

#include <otawa/ipet/TrivialInstCacheManager.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/ipet.h>
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

static SilentFeature::Maker<TrivialInstCacheManager> maker;
/**
 * This feature ensurers that the instruction cache has been modelled
 * in the IPET approach.
 */
SilentFeature INST_CACHE_SUPPORT_FEATURE("otawa::ipet::INST_CACHE_SUPPORT_FEATURE", maker);

} } // otawa::ipet
