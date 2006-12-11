/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	TrivialDataCacheManager class interface
 */
#ifndef OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H
#define OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// TrivialBBTime class
class TrivialDataCacheManager: public BBProcessor {
	FrameWork *fw;
	int time;
	void configure(FrameWork *fw);

protected:
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);

public:
	TrivialDataCacheManager(void);
};

// Features
extern Feature<TrivialDataCacheManager> DATA_CACHE_TIME_FEATURE;

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

