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
	virtual void setup(FrameWork *fw);
	virtual void cleanup(FrameWork *fw);
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);

public:
	TrivialInstCacheManager(void);
};

// Features
extern Feature<TrivialInstCacheManager> INST_CACHE_SUPPORT_FEATURE; 

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIAL_INST_CACHE_MANAGER_H
