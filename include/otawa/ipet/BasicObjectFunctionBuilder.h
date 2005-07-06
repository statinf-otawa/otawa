/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicObjectFunctionBuilder.h -- BasicObjectFunctionBuilder class interface.
 */
#ifndef OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
#define OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

class BasicObjectFunctionBuilder: public CFGProcessor {
public:
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} // otawa

#endif // OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
