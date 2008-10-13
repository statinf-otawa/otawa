/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/ipet_TrivialDataCacheManager.cpp -- TrivialDataCacheManager class implementation
 */

#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/ipet/TrivialDataCacheManager.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/ipet/TrivialBBTime.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa { namespace ipet {

/**
 * @class TrivialDataCacheManager
 * This processor apply a simple method for managing data cache in IPET method.
 * It adds to the basic block execution time the sum of penalties induced by
 * each memory access instruction in the basic block.
 */

void TrivialDataCacheManager::configure(WorkSpace *framework) {
	fw = framework;
	if(!fw->cache().hasDataCache()) {
		time = 0;
		log << "WARNING: there is no data cache here !\n";
	}
	else
		time = fw->cache().dataCache()->missPenalty();
}


/**
 * Build the trivial data cache manager.
 * @param props		Configuration properties.
 */
TrivialDataCacheManager::TrivialDataCacheManager(void)
: BBProcessor("ipet::TrivialDataCacheManager", Version(1, 0, 0)), fw(0) {
	provide(DATA_CACHE_SUPPORT_FEATURE);
	require(COLLECTED_CFG_FEATURE);
	require(BB_TIME_FEATURE);
}


/**
 */	
void TrivialDataCacheManager::processBB(WorkSpace *framework, CFG *cfg,
BasicBlock *bb) {
	assert(framework);
	assert(cfg);
	assert(bb);
	
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
 * 
 * @par Properties
 * @li @ref ipet::TIME
 */
Feature<TrivialDataCacheManager>
	DATA_CACHE_SUPPORT_FEATURE("otawa::ipet::data_cache_support");

} } // otawa::ipet
