/*
 *	$Id$
 *	Copyright (c) 2006-07, IRIT-UPS <casse@irit.fr>.
 *
 *	FullSimulationDriver.h -- FullSimulationDriver class interface.
 */
#ifndef FULL_SIMULATION_DRIVER_H
#define FULL_SIMULATION_DRIVER_H

#include <otawa/sim/Driver.h>
#include <otawa/otawa.h>

namespace otawa {

namespace sim {

// FullSimulationDriver class
class FullSimulationDriver: public Driver {
	otawa::Inst * inst;
	bool wrong_path;
	WorkSpace *fw;
	otawa::Inst *end_of_simulation;
	SimState *emulator_state;
public:
	inline FullSimulationDriver(WorkSpace *_fw, Inst * start);
	
	// Driver overload
	virtual Inst *nextInstruction(State& state, Inst *inst);
	virtual void terminateInstruction(State& state, Inst *inst);
	virtual void redirect(State &state, Inst * branch, bool direction); 
	virtual bool PredictBranch(State &state, Inst * branch, bool pred); 
};

// FullSimulationDriver inlines
inline FullSimulationDriver::FullSimulationDriver(WorkSpace *_fw, Inst * start) {
	fw = _fw;
	inst = start;
	assert(inst);
	String lab("_exit");
	end_of_simulation = fw->process()->findInstAt(lab);
	assert(end_of_simulation);
	wrong_path = false;
	emulator_state = fw->process()->newState();
	ASSERT(emulator_state);
}

} } // otawa::sim

#endif // FULL_SIMULATION_DRIVER_H
