/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <rochange@irit.fr>.
 *
 *	GenericSimulator.cpp -- GenericSimulator class implementation.
 */

#include <otawa/gensim/GenericProcessor.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/program.h>
#include <otawa/otawa.h>
#include <otawa/gensim/debug.h>

int sc_main(int argc, char *argv[]) {
	int err = dup(2);
	close(2);
	sc_core::sc_elab_and_sim(argc, argv);
	dup2(err,2);
	close(err);
	return 0;
}


namespace otawa { namespace sim {




/**
 * Instruction execution time. Default to 5.
 */
GenericIdentifier<int> INSTRUCTION_TIME("sim.instruction_time");


/**
 * @class GenericSimulator
 * The Generic simulator simulates a generic processor. 
 */


/**
 * Build an Generic simulator.
 */
GenericSimulator::GenericSimulator(void)
: Simulator("Generic_simulator", Version(1, 0, 0),
	"A generic simulator.") {
}


/**
 */	
GenericState *GenericSimulator::instantiate(FrameWork *fw, const PropList& props) {
	static GenericState* state;
	static bool initialized = false;
	if(!initialized){
		state = new GenericState(fw);
		state->init();
		initialized = true;
	}
	assert(fw == state->fw); 
	return state;
}

void GenericState::init() {
	ProcessorConfiguration conf;
	
	int degree = 4;
	int cache_line_size = 8;
	
	InstructionQueueConfiguration *fetch_queue = 
		new InstructionQueueConfiguration("FetchQueue", 4/*size = 2^4*/, NONE);
	conf.addInstructionQueue(fetch_queue);
	
//	InstructionQueueConfiguration *issue_queue = 
//		new InstructionQueueConfiguration("IssueQueue", 2/*size = 2^2*/, READY);
//	conf.addInstructionQueue(issue_queue);

	InstructionQueueConfiguration * rob = 
		new InstructionQueueConfiguration("ROB", 5/*size = 2^5*/, EXECUTED);
	conf.addInstructionQueue(rob);
	
	
	PipelineStageConfiguration * fetch_stage = 
		new PipelineStageConfiguration("FetchStage", FETCH, NULL, fetch_queue, cache_line_size);
	conf.addPipelineStage(fetch_stage);
	
	PipelineStageConfiguration * decode_stage = 
		new PipelineStageConfiguration("DecodeStage", LAZYIQIQ, fetch_queue, rob, degree);
	conf.addPipelineStage(decode_stage);
	
//	PipelineStageConfiguration * execute_stage = 
//		new PipelineStageConfiguration("ExecuteStage", EXECUTE_IN_ORDER, issue_queue, NULL, degree);
//	conf.addPipelineStage(execute_stage);

	PipelineStageConfiguration * execute_stage = 
		new PipelineStageConfiguration("ExecuteStage", EXECUTE_OUT_OF_ORDER, rob, degree);
	conf.addPipelineStage(execute_stage);

	PipelineStageConfiguration * commit_stage = 
		new PipelineStageConfiguration("CommitStage", COMMIT, rob, NULL, degree);
	conf.addPipelineStage(commit_stage);
	
	
	processor = new GenericProcessor("GenericProcessor",&conf, this, fw->platform());

}



void GenericState::step(void) {
	TRACE(elm::cout << "Cycle " << _cycle << "\n";)
	processor->step();
	_cycle ++;
	running = ! processor->isEmpty();
}


} } // otawa::sim

