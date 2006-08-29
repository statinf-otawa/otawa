/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_BasicBlockDriver.cpp -- BasicBlockDriver class implementation.
 */

#include <otawa/sim/State.h>
#include <otawa/sim/BasicBlockDriver.h>

namespace otawa { namespace sim {

/**
 * @class BasicBlockDriver <otawa/sim/BasicBlockDriver.h>
 * This class is a simulator @ref Driver that executes the instruction sequence
 * found in a basic block.
 */


/**
 * @fn BasicBlockDriver::BasicBlockDriver(BasicBlock *bb);
 * BasicBlockDriver constructor.
 * @param bb	Basic block to simulate.
 */


/**
 */
Inst *BasicBlockDriver::nextInstruction(State& state, Inst *_inst) {
	if(inst.ended()) {
		state.stop();
		return 0;
	}
	else {
		Inst *result = *inst;
		inst.next();
		return result;
	}
}


/**
 */
void BasicBlockDriver::terminateInstruction(State& state, Inst *inst) {
}

} }	// otawa::sim
