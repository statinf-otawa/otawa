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

// External classes	
class BasicBlock;
namespace ilp {
	class System;
} //ilp


// BasicConstraintsBuilder class
class BasicConstraintsBuilder: public CFGProcessor {
	void make(ilp::System *system, BasicBlock *bb);
public:
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} // otawa

#endif 	// OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
