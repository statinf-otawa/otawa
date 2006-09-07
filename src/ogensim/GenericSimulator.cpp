/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <rochange@irit.fr>.
 *
 *	GenericSimulator.cpp -- GenericSimulator class implementation.
 */

#include <otawa/gensim/GenericProcessor.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/gensim/GenericState.h>
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



namespace otawa {

GenericIdentifier<int> DEGREE("gensim.degree", 1);
	
namespace sim {




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
State *GenericSimulator::instantiate(FrameWork *fw, const PropList& props) {
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
	
	int degree = DEGREE(fw->process());
	int cache_line_size = 8;

	// config. 3 stages, in-order execution
	
	#if 0
	InstructionQueueConfiguration *fetch_queue = 
		new InstructionQueueConfiguration("FetchQueue", degree + 1, NONE);
	conf.addInstructionQueue(fetch_queue);
	
	InstructionQueueConfiguration *issue_queue = 
		new InstructionQueueConfiguration("IssueQueue", degree + 1, READY);
	conf.addInstructionQueue(issue_queue);

	PipelineStageConfiguration * fetch_stage = 
		new PipelineStageConfiguration("FetchStage", FETCH, NULL, fetch_queue, cache_line_size);
	conf.addPipelineStage(fetch_stage);
	
	PipelineStageConfiguration * decode_stage = 
		new PipelineStageConfiguration("DecodeStage", LAZYIQIQ, fetch_queue, issue_queue, degree);
	conf.addPipelineStage(decode_stage);
	
	PipelineStageConfiguration * execute_stage = 
		new PipelineStageConfiguration("ExecuteStage", EXECUTE_IN_ORDER, issue_queue, NULL, degree);
	conf.addPipelineStage(execute_stage);
	#endif

	// config. 	5 stages, ooo execution
	
	#if 1
	InstructionQueueConfiguration *fetch_queue = 
		new InstructionQueueConfiguration("FetchQueue", degree + 1, NONE);
	conf.addInstructionQueue(fetch_queue);
	
	InstructionQueueConfiguration * rob = 
		new InstructionQueueConfiguration("ROB", degree + 4, EXECUTED);
	conf.addInstructionQueue(rob);
		
	PipelineStageConfiguration * fetch_stage = 
		new PipelineStageConfiguration("FetchStage", FETCH, NULL, fetch_queue, cache_line_size);
	conf.addPipelineStage(fetch_stage);
	
	PipelineStageConfiguration * decode_stage = 
		new PipelineStageConfiguration("DecodeStage", LAZYIQIQ, fetch_queue, rob, 1 << degree);
	conf.addPipelineStage(decode_stage);
	
	PipelineStageConfiguration * execute_stage = 
		new PipelineStageConfiguration("ExecuteStage", EXECUTE_OUT_OF_ORDER, rob, 1 << degree);
	conf.addPipelineStage(execute_stage);

	PipelineStageConfiguration * commit_stage = 
		new PipelineStageConfiguration("CommitStage", COMMIT, rob, NULL, 1 << degree);
	conf.addPipelineStage(commit_stage);
	#endif
	
	FunctionalUnitConfiguration * functional_unit =
		new FunctionalUnitConfiguration(true, 5, 1);
	functional_unit->addInstructionType(LOAD);
	functional_unit->addInstructionType(STORE);
	conf.addFunctionalUnit(functional_unit);
	
	functional_unit =
		new FunctionalUnitConfiguration(false, 1, 2);
	functional_unit->addInstructionType(COND_BRANCH);
	functional_unit->addInstructionType(UNCOND_BRANCH);
	functional_unit->addInstructionType(CALL);
	functional_unit->addInstructionType(RETURN);
	functional_unit->addInstructionType(TRAP);
	functional_unit->addInstructionType(IALU);	
	functional_unit->addInstructionType(OTHER);	
	conf.addFunctionalUnit(functional_unit);
	
	functional_unit =
		new FunctionalUnitConfiguration(true, 3, 1);
	functional_unit->addInstructionType(FALU);

	functional_unit =
		new FunctionalUnitConfiguration(true, 6, 1);
	functional_unit->addInstructionType(MUL);
	conf.addFunctionalUnit(functional_unit);

	functional_unit =
		new FunctionalUnitConfiguration(false, 15, 1);
	functional_unit->addInstructionType(DIV);
	conf.addFunctionalUnit(functional_unit);
	
	processor = new GenericProcessor("GenericProcessor",&conf, this, fw->platform());

}



void GenericState::step(void) {
	TRACE(elm::cout << "Cycle " << _cycle << "\n";)
	processor->step();
	_cycle ++;
	running = ! processor->isEmpty();
}


} } // otawa::sim

