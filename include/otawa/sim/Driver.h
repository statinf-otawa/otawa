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
	virtual Inst *nextInstruction(State& state, Inst *inst) = 0;
	virtual void terminateInstruction(State& state, Inst *inst) = 0;
};

} } // otawa::sim

#endif	// OTAWA_SIM_DRIVER_H
