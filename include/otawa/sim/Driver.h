/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/Driver.h -- Driver class interface.
 */
#ifndef OTAWA_SIM_DRIVER_H
#define OTAWA_SIM_DRIVER_H

#include <otawa/base.h>

namespace otawa {

// External classes
class Inst;
	
namespace sim {

// External classes
class State;

// Driver class
class Driver {
public :
	virtual ~Driver(void);
	
	// control flow
	virtual Inst *nextInstruction(State& state, Inst *inst) = 0;
	virtual void terminateInstruction(State& state, Inst *inst) = 0;
	
	// memory accesses
	virtual Address lowerRead(void);
	virtual Address upperRead(void);
	virtual Address lowerWrite(void);
	virtual Address upperWrite(void);

	// !!TODO!! remove them
	virtual void redirect(State &state, Inst * branch, bool direction) {
	};
	virtual bool PredictBranch(State &state, Inst * branch, bool pred) {
		return true;  // branch correctly predicted
	};
};

} } // otawa::sim

#endif	// OTAWA_SIM_DRIVER_H
