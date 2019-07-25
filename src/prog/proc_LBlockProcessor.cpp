/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	src/prog/proc_LBlockProcessor.cpp -- LBlockProcessor class implementation.
 */

#include <otawa/proc/LBlockProcessor.h>
#include <otawa/otawa.h>
#include <otawa/cache/LBlockBuilder.h>
#include <otawa/hard/CacheConfiguration.h>

namespace otawa { namespace cache {

/**
 * @class LBlockProcessor
 * This is a specialization of the processor class dedicated to LBlock
 *
 * @note This processor automatically call @ref LBlockBuilder
 * @ingroup proc
 */

p::declare LBlockProcessor::reg = p::init("otawa::LBlockProcessor", Version(1, 0, 0))
	.require(cache::COLLECTED_LBLOCKS_FEATURE)
	.require(hard::CACHE_CONFIGURATION_FEATURE);


/**
 * Build a new named processor.
 * @param name		Processor name.
 * @param version	Processor version.
 */
LBlockProcessor::LBlockProcessor(AbstractRegistration &registration): Processor(registration), _cache(nullptr) {
}


/**
 */
void LBlockProcessor::processWorkSpace(WorkSpace *ws) {

	// get configuration
	LBlockSet **sets = LBLOCKS(ws);
	ASSERT(sets);
	const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION_FEATURE.get(ws);
	ASSERT(conf);
	_cache = conf->instCache();
	ASSERT(_cache);

	// process the sets
	for(int i = 0; i < _cache->rowCount(); i++) {
		if(logFor(LOG_CFG))
			log << "\tLBLOCKSET " << sets[i]->line() << io::endl;
		processLBlockSet(ws, sets[i]);
	}
}


/**
 * Called for each LBlock set.
 * @param ws	Current workspace.
 * @param set	Current LBlock set.
 */
void LBlockProcessor::processLBlockSet(WorkSpace *ws, LBlockSet *set) {
	for(LBlockSet::Iterator lb(*set); lb(); lb++)
		processLBlock(ws, set, *lb);
}

/**
 * Called for each LBlock.
 * @param ws		Current workspace.
 * @param set		Current LBlock set.
 * @param lblock	Current LBlock.
 */
void LBlockProcessor::processLBlock(WorkSpace *ws, LBlockSet *set, LBlock *lblock) {
}

} }	// otawa::cache
