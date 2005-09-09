/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/TrivialDataCacheManager.h -- TrivialDataCacheManager class interface
 */
#ifndef OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H
#define OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace ipet {

// TrivialBBTime class
class TrivialDataCacheManager: public BBProcessor {
	FrameWork *fw;
	int time;
	void configure(FrameWork *fw);
public:
	TrivialDataCacheManager(const PropList& props = PropList::EMPTY);
	
	// BBProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

