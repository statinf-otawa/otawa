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
public:
	static p::declare reg;
	TrivialDataCacheManager(p::declare& r = reg);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *fw, CFG *cfg, Block *bb);

private:
	int time;
};

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIAL_DATA_CACHE_MANAGER_H

