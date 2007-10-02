/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/util/LBlockBuilder.h -- interface of LBlockBuilder class.
 */
#ifndef OTAWA_UTIL_LBLOCKBUILDER_H
#define OTAWA_UTIL_LBLOCKBUILDER_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/proc/Feature.h>
#include <elm/genstruct/Vector.h> 

namespace otawa {

namespace hard {
	class Cache;
}

// LBlockBuilder class
class LBlockBuilder: public CFGProcessor {
	LBlockSet **lbsets;
	const hard::Cache *cache;
	HashTable<int,int> *cacheBlocks;
	void processLBlockSet(WorkSpace *fw, CFG *cfg, LBlockSet *lbset, const hard::Cache *cach, int *tableindex);

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
	virtual void cleanup(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);

public:
	LBlockBuilder(void);
};

// Features
extern Feature<LBlockBuilder> COLLECTED_LBLOCKS_FEATURE;

} // otawa

#endif // OTAWA_UTIL_LBLOCKBUILDER_H
