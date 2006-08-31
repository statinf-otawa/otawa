/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	BBTimeSimulator.cpp -- BBTimeSimulator class implementation.
 */
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/ipet/IPET.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/sim/BasicBlockDriver.h>

using namespace otawa::sim;

namespace otawa { namespace ipet {

BBTimeSimulator::BBTimeSimulator(const PropList& props)
: BBProcessor("otawa::BBTimeSimulator", Version(0, 1, 0), props) {
}

/**
 * Simulates the execution time of the given basic block, using
 * the GenericSimulator
 */
void BBTimeSimulator::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb){
	GenericSimulator simulator;
	sim::State *state = simulator.instantiate(fw);
	BasicBlockDriver driver(bb);
	state->reset();
	state->run(driver);
	TIME(bb) = state->cycle();
}

} }
