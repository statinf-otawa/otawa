#include "BBTimeSimulator.h"
#include <otawa/ipet/IPET.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/sim/BasicBlockDriver.h>

using namespace otawa::sim;

namespace otawa { namespace ipet {

BBTimeSimulator::BBTimeSimulator(const PropList& props)
: BBProcessor("otawa::BBTimeSimulator", Version(0, 1, 0), props) {
}

void BBTimeSimulator::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb){
	if(bb->countInstructions() == 0){
		TIME(bb) = 0;
	}
	else if(dynamic_cast<SequenceBasicBlock*>(bb)){
		processSequenceBB(fw, static_cast<SequenceBasicBlock*>(bb));
	}
	else{
		processNormalBB(fw, bb);
	}
}

void BBTimeSimulator::processSequenceBB(FrameWork *fw, SequenceBasicBlock *bb){
	int time = bb->getBBPath()->time(fw);
	TIME(bb) = time;
}


void BBTimeSimulator::processNormalBB(FrameWork *fw, BasicBlock *bb){
	int start, end;
	static bool initialized = false;
	if(!initialized){
		sc_core::sc_elab_and_sim(0, NULL);
		initialized = true;
	}
	static GenericSimulator simulator;
	static State *state = simulator.instantiate(fw);
	BasicBlockDriver driver(bb);
	start = state->cycle();
	state->run(driver);
	end = state->cycle();
	TIME(bb) = end - start;
	//delete state;
}

} }
