/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	TrivialDataCacheManager class interface
 */
#ifndef OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H
#define OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// TrivialBBTime class
class TrivialDataCacheManager: public BBProcessor {
	WorkSpace *fw;
	int time;
	void configure(WorkSpace *fw);

protected:
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);

public:
	TrivialDataCacheManager(void);
};

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

