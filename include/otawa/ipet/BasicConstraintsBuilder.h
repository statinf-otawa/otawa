/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicConstraintsBuilder.h -- BasicConstraintsBuilder class interface.
 */
#ifndef OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
#define OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// External classes	
class BasicBlock;
namespace ilp {
	class System;
} //ilp

namespace ipet {
	
// BasicConstraintsBuilder class
class BasicConstraintsBuilder: public BBProcessor {
public:
	BasicConstraintsBuilder(void);

	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};

// Features
extern Feature <BasicConstraintsBuilder> CONTROL_CONSTRAINTS_FEATURE;

} } // otawa::ipet

#endif 	// OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
