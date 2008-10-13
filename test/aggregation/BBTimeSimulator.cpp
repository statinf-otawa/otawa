/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	BBTimeSimulator.cpp -- BBTimeSimulator class implementation.
 */
#include "BBTimeSimulator.h"
#include <otawa/ipet/IPET.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/sim/BasicBlockDriver.h>

using namespace otawa::sim;

namespace otawa { namespace ipet {

/**
 * @author G. Cavaignac
 * @class BBTimeSimulator
 * This basic block processor is used to simulate execution times
 * of basic blocks (SequenceBasicBlocks too)
 * 
 * This class is temporary, because SequenceBasicBlock is a class
 * that will be removes in the future
 */


/**
 * Constructs a new BBTimeSimulator
 */
BBTimeSimulator::BBTimeSimulator(const PropList& props)
: BBProcessor("otawa::BBTimeSimulator", Version(0, 1, 0), props) {
}

/**
 * Gives a TIME identifier to the given basic block.
 * Calls @ref processSequenceBB or @ref processNormalBB according to
 * the type of the given basic block
 */
void BBTimeSimulator::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb){
	if(bb->countInsts() == 0){
		TIME(bb) = 0;
	}
	else if(dynamic_cast<SequenceBasicBlock*>(bb)){
		processSequenceBB(fw, static_cast<SequenceBasicBlock*>(bb));
	}
	else{
		processNormalBB(fw, bb);
	}
}


/**
 * Gives a TIME identifier to the given SequenceBasicBlock.
 * This method calls the @ref BBPath::time() method to simulate
 */
void BBTimeSimulator::processSequenceBB(FrameWork *fw, SequenceBasicBlock *bb){
	int time = bb->getBBPath()->time(fw);
	TIME(bb) = time;
}

/**
 * Gives a TIME identifier to the given standard BasicBlock
 */
void BBTimeSimulator::processNormalBB(FrameWork *fw, BasicBlock *bb){
	static GenericSimulator simulator;
	static State *state = simulator.instantiate(fw);
	BasicBlockDriver driver(bb);
	state->reset();
	state->run(driver);
	TIME(bb) = state->cycle();
}

} }
