/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicConstraintsBuilder.h -- BasicConstraintsBuilder class interface.
 */
#ifndef OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
#define OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {
	
class BasicConstraintsBuilder: public CFGProcessor {
public:
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} // otawa

#endif 	// OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
