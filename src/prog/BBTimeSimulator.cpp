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
: BBProcessor("otawa::BBTimeSimulator", Version(1, 0, 0)) {
	provide(BB_TIME_FEATURE);
}

/**
 */
void BBTimeSimulator::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb){
	BasicBlockDriver driver(bb);
	state->reset();
	state->run(driver);
	TIME(bb) = state->cycle();
	//elm::cout << "BB " << INDEX(bb) << ": " << state->cycle() << io::endl;
}


/**
 */
void BBTimeSimulator::setup(WorkSpace *ws) {
	Simulator *simulator = ws->process()->simulator();
	if(!simulator)
		throw ProcessorException(*this, "no simulator available");
	state = simulator->instantiate(ws);
}


/**
 */
void BBTimeSimulator::cleanup(WorkSpace *ws) {
	// delete state; !!TODO!!
	state = 0;
}

} }
