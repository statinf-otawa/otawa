/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_FullSimulationDriver.cpp -- FullSimulationDriver class implementation.
 */

#include <otawa/sim/State.h>
#include <otawa/prog/Inst.h>
#include "FullSimulationDriver.h"

#define TRACE(msg)	//cerr << msg << io::endl;

using namespace otawa;

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
Inst * FullSimulationDriver::nextInstruction(State& state, Inst *_inst)
{
	// if you consider only startup stripped executables,
	// the last instruction to execute is a sc which is 2 instr. after _start.
	// startup sequence seems to be always (in H. casse's benchs):
	// 	_start:
	// 		bl main
	// 		li r0,1
	// 		sc
	// afterwards, there is the program (main + others subroutine)
	// so we should stop simulation on the sc
	// (stop fetching in reality, but continue sim until all instructions complete)
	
	if (inst == end_of_simulation)
	{
		TRACE("last instruction fetched, cycle " << state.cycle())
		return 0;
		// no more instructions will be fetched so the simulator will
		// normally stop when all intructions complete and the processor gets empty
	}
	else {
		TRACE("EXEC " << inst->address() << ":" << inst);
		if (! wrong_path)
			inst = emulator_state->execute(inst);
		return inst;
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
		inst = branch->nextInst();
	wrong_path = false;
}



/**
 */
void FullSimulationDriver::terminateInstruction(State& state, Inst *inst) {
}


} }	// otawa::sim
