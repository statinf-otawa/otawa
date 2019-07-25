/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/BasicBlockDriver.h -- BasicBlockDriver class interface.
 */
#ifndef OTAWA_SIM_BASICBLOCK_DRIVER_H
#define OTAWA_SIM_BASICBLOCK_DRIVER_H

#include <otawa/sim/Driver.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// Extern class
class BasicBlock;
	
namespace sim {

// BasicBlockDriver class
class BasicBlockDriver: public Driver {
	BasicBlock::InstIter inst;
public:
	inline BasicBlockDriver(BasicBlock *bb);
	
	// Driver overload
	virtual Inst *nextInstruction(State& state, Inst *inst);
	virtual void terminateInstruction(State& state, Inst *inst);
};

// BasicBlockDriver inlines
inline BasicBlockDriver::BasicBlockDriver(BasicBlock *bb): inst(bb) {
	ASSERT(bb);
}

} } // otawa::sim

#endif // OTAWA_SIM_BASICBLOCK_DRIVER_H
