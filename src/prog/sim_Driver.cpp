/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/Driver.h -- Driver class interface.
 */

#include <otawa/sim/Driver.h>

namespace otawa { namespace sim {

/**
 * @class Driver <otawa/sim.h>
 * This class is used to drive a simulator. The simulator will call methods
 * of this class in order to get next instruction to execution or to inform it
 * about a finishing instruction.
 */


/**
 */
Driver::~Driver(void) {
}


/**
 * @fn Inst *Driver::nextInstruction(State& state, Inst *inst);
 * This method is called each time the simulator needs the next instruction to
 * execute.
 * @param state	Current state of the simulator.
 * @param inst	Next instruction proposed by the simulator. If the simulator does
 * not handle the semantics of instructions, it is juste the next instruction.
 * @return	Next instruction to execute or null if there is no more instruction
 * to execute.
 */


/**
 * @fn void Driver::terminateInstruction(State& state, Inst *inst);
 * This method is called when an instruction is terminated.
 * @param stat	Current state of the simulator.
 * @param inst	Terminated instruction.
 */
	
} } // otawa::sim
