#include "BBTimeSimulator.h"
#include <otawa/ipet/IPET.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/sim/BasicBlockDriver.h>

using namespace otawa::sim;

namespace otawa { namespace ipet {

BBTimeSimulator::BBTimeSimulator(const PropList& props)
: BBProcessor("otawa::BBTimeSimulator", Version(1, 0, 0), props) {
}

void BBTimeSimulator::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb){
	GenericSimulator simulator;
	sim::State *state = simulator.instantiate(fw);
	BasicBlockDriver driver(bb);
	state->reset();
	state->run(driver);
	TIME(bb) = state->cycle();
}

} }
