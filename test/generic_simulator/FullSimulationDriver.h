/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	FullSimulationDriver.h -- FullSimulationDriver class interface.
 */
#ifndef FULL_SIMULATION_DRIVER_H
#define FULL_SIMULATION_DRIVER_H

#include <otawa/sim/Driver.h>
//#include <otawa/cfg/BasicBlock.h>
#include <emul.h>
#include <iss_include.h>

namespace otawa {

// Extern class
//class BasicBlock;
	
namespace sim {

// FullSimulationDriver class
class FullSimulationDriver: public Driver {
	otawa::Inst * inst;
	state_t emulator_state;
	bool wrong_path;
public:
	inline FullSimulationDriver(Inst * start, state_t * init_state);
	
	// Driver overload
	virtual Inst *nextInstruction(State& state, Inst *inst);
	virtual void terminateInstruction(State& state, Inst *inst);
	virtual void redirect(State &state, Inst * branch, bool direction); 
	virtual bool PredictBranch(State &state, Inst * branch, bool pred); 
};

// FullSimulationDriver inlines
inline FullSimulationDriver::FullSimulationDriver(Inst * start, state_t * init_state) {
	inst = start;
	assert(inst);
	wrong_path = false;
	memcpy(&emulator_state, init_state, sizeof(state_t));
}

} } // otawa::sim

#endif // FULL_SIMULATION_DRIVER_H
