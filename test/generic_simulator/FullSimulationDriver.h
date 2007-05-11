/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	FullSimulationDriver.h -- FullSimulationDriver class interface.
 */
#ifndef FULL_SIMULATION_DRIVER_H
#define FULL_SIMULATION_DRIVER_H

#include <otawa/sim/Driver.h>
#include <otawa/otawa.h>
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
	FrameWork *fw;
	otawa::Inst *end_of_simulation;
public:
	inline FullSimulationDriver(FrameWork *_fw, Inst * start, state_t * init_state);
	
	// Driver overload
	virtual Inst *nextInstruction(State& state, Inst *inst);
	virtual void terminateInstruction(State& state, Inst *inst);
	virtual void redirect(State &state, Inst * branch, bool direction); 
	virtual bool PredictBranch(State &state, Inst * branch, bool pred); 
};

// FullSimulationDriver inlines
inline FullSimulationDriver::FullSimulationDriver(FrameWork *_fw, Inst * start, state_t * init_state) {
	fw = _fw;
	inst = start;
	assert(inst);
	// set end of simulation 2 instr. after _start
	//end_of_simulation = fw->findInstAt(inst->address() + 2*sizeof(code_t));
	String lab("_exit");
	end_of_simulation = fw->process()->findInstAt(lab); //inst->address() + 2*sizeof(code_t));
	assert(end_of_simulation);
	wrong_path = false;
	memcpy(&emulator_state, init_state, sizeof(state_t));
}

} } // otawa::sim

#endif // FULL_SIMULATION_DRIVER_H
