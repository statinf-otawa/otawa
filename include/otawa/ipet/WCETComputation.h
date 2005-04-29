/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/WCETComputation.h -- WCETComputation class interface.
 */
#ifndef OTAWA_IPET_WCET_COMPUTATION_H
#define OTAWA_IPET_WCET_COMPUTATION_H

#include <otawa/proc/CFGProcessor.h>

namespace otawa {
	
class WCETComputation: public CFGProcessor {
public:

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} // otawa

#endif	// OTAWA_IPET_WCET_COMPUTATION_H
