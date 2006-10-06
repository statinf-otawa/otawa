/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/WCETComputation.h -- WCETComputation class interface.
 */
#ifndef OTAWA_IPET_WCET_COMPUTATION_H
#define OTAWA_IPET_WCET_COMPUTATION_H

#include <otawa/proc/CFGProcessor.h>

namespace otawa { namespace ipet {

// WCETComputation class
class WCETComputation: public Processor {
public:
	WCETComputation(const PropList& props = PropList::EMPTY);

	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
};

} } // otawa::ipet

#endif	// OTAWA_IPET_WCET_COMPUTATION_H
