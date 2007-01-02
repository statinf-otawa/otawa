/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	BBTimeSimulator.cpp -- BBTimeSimulator class implementation.
 */
#include <otawa/ipet/BBTimeSimulator.h>
#include <otawa/ipet/IPET.h>
#include <otawa/sim.h>
#include <otawa/sim/BasicBlockDriver.h>
#include <otawa/ipet/TrivialBBTime.h>

using namespace otawa::sim;

namespace otawa { namespace ipet {

/**
 * This processor compute the execution time of each basic block using the
 * provided simulator.
 * 
 * @par Provided Features
 * @li @ref otawa::ipet::BB_TIME_FEATURE
 */
BBTimeSimulator::BBTimeSimulator(void)
: BBProcessor("otawa::BBTimeSimulator", Version(0, 1, 0)) {
	require(BB_TIME_FEATURE);
}

/**
 */
void BBTimeSimulator::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb){
	Simulator *simulator = fw->process()->simulator();
	if(!simulator)
		throw ProcessorException(*this, "no simulator available");
	sim::State *state = simulator->instantiate(fw);
	BasicBlockDriver driver(bb);
	state->reset();
	state->run(driver);
	TIME(bb) = state->cycle();
	//elm::cout << "BB " << INDEX(bb) << ": " << state->cycle() << io::endl;
}

} }
