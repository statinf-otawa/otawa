/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/WCETComputation.h -- WCETComputation class interface.
 */
#ifndef OTAWA_IPET_WCET_COMPUTATION_H
#define OTAWA_IPET_WCET_COMPUTATION_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// WCETComputation class
class WCETComputation: public Processor {
protected:
	virtual void processFrameWork(FrameWork *fw);
public:
	WCETComputation(void);
};

// Features
extern Feature<WCETComputation> WCET_FEATURE;

} } // otawa::ipet

#endif	// OTAWA_IPET_WCET_COMPUTATION_H
