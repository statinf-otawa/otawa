/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	TrivialInstCacheManager class interface
 */
#ifndef OTAWA_IPET_TRIVIAL_INST_CACHE_MANAGER_H
#define OTAWA_IPET_TRIVIAL_INST_CACHE_MANAGER_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

namespace hard {
	class Cache;
}
	
namespace ipet {

// TrivialInstCacheManager class
class TrivialInstCacheManager: public BBProcessor {
	const hard::Cache *cache;
protected:
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);

public:
	TrivialInstCacheManager(void);
};

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIAL_INST_CACHE_MANAGER_H
