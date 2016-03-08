/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/util/PostDominance.h -- PostDominance class interface.
 */
#ifndef OTAWA_UTIL_POSTDOMINANCE_H
#define OTAWA_UTIL_POSTDOMINANCE_H

#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

class PostDominance: public CFGProcessor {
public:
	static p::declare reg;
	PostDominance(p::declare& r = reg);

protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
};

} // otawa

#endif // OTAWA_UTIL_POSTDOMINANCE_H
