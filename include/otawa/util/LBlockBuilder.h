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

namespace otawa {

namespace hard {
	class Cache;
}

// LBlockBuilder class
class LBlockBuilder: public CFGProcessor {
	LBlockSet **lbsets;
	const hard::Cache *cache;
	void processLBlockSet(WorkSpace *fw, CFG *cfg, LBlockSet *lbset);

protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void processCFG(WorkSpace *fw, CFG *cfg);

public:
	LBlockBuilder(void);
};

// Features
extern Feature<LBlockBuilder> COLLECTED_LBLOCKS_FEATURE;

} // otawa

#endif // OTAWA_UTIL_LBLOCKBUILDER_H
