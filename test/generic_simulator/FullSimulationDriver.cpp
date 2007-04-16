/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_FullSimulationDriver.cpp -- FullSimulationDriver class implementation.
 */

#include <otawa/sim/State.h>
#include <FullSimulationDriver.h>
#include <otawa/prog/Inst.h>
#include <otawa/gensim/debug.h>

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
		TRACE(elm::cout << "last instruction fetched, cycle " << state.cycle() << "\n";)
		return 0;
		// no more instructions will be fetched so the simulator will
		// normally stop when all intructions complete and the processor gets empty
	}
	else {
		Inst *result = inst;
		if (! wrong_path)
		{
			code_t code;
			iss_fetch((::address_t)(unsigned long)inst->address(), &code);
			instruction_t *emulated_inst = iss_decode(&emulator_state, (::address_t)(unsigned long)inst->address(), &code, 0);
			iss_complete(emulated_inst, &emulator_state);
			iss_free(emulated_inst);
			if (NIA(&emulator_state) != CIA(&emulator_state) + sizeof(code_t))
			{
				if (inst->target())
					inst = inst->target();
				else
					// target instr. is not set for dynamic branch such as returns and equivalent
					// so we must find it ourself
					inst = fw->findInstAt((address_t)((unsigned long)(NIA(&emulator_state))));
			}
			else
				inst = inst->nextInst(); // see if next() could return 0
		}
		
		// pseudo instruction are put before the beginning of basic blocks
		// we mustn't simulate such instructions, the next real instr.
		// is inst->next()
		while (inst->isPseudo())
			inst = inst->nextInst();

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
