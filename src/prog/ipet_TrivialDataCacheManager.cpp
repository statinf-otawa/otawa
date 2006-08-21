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

namespace otawa { namespace ipet {

/**
 * @class TrivialDataCacheManager
 * This processor apply a simple method for managing data cache in IPET method.
 * It adds to the basic block execution time the sum of penalties induced by
 * each memory access instruction in the basic block.
 */

void TrivialDataCacheManager::configure(FrameWork *framework) {
	fw = framework;
	if(!fw->cache().hasDataCache())
		time = 0;
	else
		time = fw->cache().dataCache()->missPenalty();
}


/**
 * Build the trivial data cache manager.
 * @param props		Configuration properties.
 */
TrivialDataCacheManager::TrivialDataCacheManager(const PropList& props)
: BBProcessor("ipet::TrivialDataManager", Version(1, 0, 0), props) {
}


/**
 */
void TrivialDataCacheManager::processCFG(FrameWork *framework, CFG *cfg) {
	configure(framework);
	BBProcessor::processCFG(framework, cfg);
}


/**
 */	
void TrivialDataCacheManager::processBB(FrameWork *framework, CFG *cfg,
BasicBlock *bb) {
	assert(fw);
	assert(cfg);
	assert(bb);
	
	// Check configuration
	if(framework != fw)
		configure(framework);
	
	// Do not process entry and exit
	if(bb->isEntry() || bb->isExit())
		return;
	
	// Count the memory accesses
	int count = 0;
	for(Iterator<Inst*> inst(bb->visit()); inst; inst++)
		if(inst->isMem())
			count++;
	
	// Store new BB time
	int bb_time = bb->get<int>(TIME, 0);
	bb_time += count * time;
	bb->set(TIME, bb_time);
}

} } // otawa::ipet
