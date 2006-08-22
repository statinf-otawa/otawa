/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <rochange@irit.fr>.
 *
 *	GenericSimulator.cpp -- GenericSimulator class implementation.
 */

#include <GenericProcessor.h>
#include <GenericSimulator.h>
#include <otawa/program.h>
#include <otawa/otawa.h>

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
	return new GenericState(fw, 5);
}

void GenericState::init() {
	ProcessorConfiguration conf;
	
	int degree = 4;
	int cache_line_size = 8;
	
	InstructionQueueConfiguration *fetch_queue = 
		new InstructionQueueConfiguration("FetchQueue", 4/*size = 2^4*/);
	conf.addInstructionQueue(fetch_queue);
	
	InstructionQueueConfiguration *issue_queue = 
		new InstructionQueueConfiguration("IssueQueue", 2/*size = 2^2*/);
	conf.addInstructionQueue(issue_queue);
	
	
//	InstructionQueueConfiguration *rob = 
//		new InstructionQueueConfiguration("ROB", 6 /*size = 64*/);
//	conf.addInstructionQueue(rob);
	
	PipelineStageConfiguration * fetch_stage = 
		new PipelineStageConfiguration("FetchStage", FETCH, NULL, fetch_queue, cache_line_size);
	conf.addPipelineStage(fetch_stage);
	
	PipelineStageConfiguration * decode_stage = 
		new PipelineStageConfiguration("DecodeStage", LAZYIQIQ, fetch_queue, issue_queue, degree);
	conf.addPipelineStage(decode_stage);
	
	PipelineStageConfiguration * execute_stage = 
		new PipelineStageConfiguration("ExecuteStage", EXECUTE_IN_ORDER, issue_queue, NULL, degree);
	conf.addPipelineStage(execute_stage);
	
	
	processor = new GenericProcessor("GenericProcessor",&conf, this);

}



void GenericState::step(void) {
	elm::cout << "Cycle " << _cycle << "\n";
	processor->step();
	_cycle ++;
	running = ! processor->isEmpty();
}


} } // otawa::sim

