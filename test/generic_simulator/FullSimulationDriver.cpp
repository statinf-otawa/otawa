/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_FullSimulationDriver.cpp -- FullSimulationDriver class implementation.
 */

#include <otawa/sim/State.h>
#include <FullSimulationDriver.h>
#include <otawa/prog/Instruction.h>

namespace otawa { namespace sim {

/**
 * @class FullSimulationDriver <FullSimulationDriver.h>
 * This class is a simulator @ref Driver that executes the complete program.
 */


/**
 * @fn FullSimulationDriver::FullSimulationDriver();
 * FullSimulationDriver constructor.
 */


/**
 */
Inst * FullSimulationDriver::nextInstruction(State& state, Inst *_inst) {
	if(inst->next() == NULL) {
		state.stop();
		return 0;
	}
	else {
		Inst *result = inst;
		if (! wrong_path) {
			code_t code;
			iss_fetch((::address_t)(unsigned long)inst->address(), &code);
			instruction_t *emulated_inst = iss_decode(&emulator_state, (::address_t)(unsigned long)inst->address(), &code);
			iss_complete(emulated_inst,&emulator_state);
			iss_free(emulated_inst);
			if (NIA(&emulator_state) != CIA(&emulator_state) + sizeof(code_t))
				inst = inst->target();
			else
				inst = inst->next();
		}
		return result;
	}
}

/**
 */

bool FullSimulationDriver::PredictBranch(State &state, Inst *branch, bool pred) {
	if  ( 	( (pred==true) && (inst == branch->target()) )
		||
			( (pred==false) && (inst == branch->next())) ) {
		return true; // branch well predicted
	}
	// branch mispredicted
	wrong_path = true;
	return false;
}


/**
 */

void FullSimulationDriver::redirect(State &state, Inst * branch, bool direction) {
	if (direction == true)
		inst = branch->target();
	else
		inst = branch->next();
	wrong_path = false;
}



/**
 */
void FullSimulationDriver::terminateInstruction(State& state, Inst *inst) {
}


} }	// otawa::sim
