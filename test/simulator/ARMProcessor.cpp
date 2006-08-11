#include <ARMProcessor.h>
#include <ARMSimulator.h>


void ARMProcessor::build(otawa::ARMState *state) {
	
	sim_state = state;
	// state_t *emul_state = GLISS_STATE(Fw);
	
	elm::cout << "Building the processor ...";
	fetch_stage = new FetchStage("FetchStage");
	fetch_stage->configure(1,1);
	fetch_stage->init(state);
 	decode_stage = new DecodeStage("DecodeStage");
 	decode_stage->configure(1);
  	execute_stage = new ExecuteStage("ExecuteStage");
  	execute_stage->configure(1);
  
  	fetch_stage->clock(clock);
  	fetch_stage->number_of_accepted_instructions(number_of_accepted_fetched_instructions);
  	fetch_stage->number_of_leaving_instructions(number_of_fetched_instructions);
  	for (int i=0 ; i<MAX_WIDTH ; i++)
    	fetch_stage->fetched_instruction[i](fetched_instruction[i]);
    	
    decode_stage->clock(clock);
    decode_stage->number_of_accepted_fetched_instructions(number_of_accepted_fetched_instructions);
    decode_stage->number_of_fetched_instructions(number_of_fetched_instructions);
    decode_stage->number_of_accepted_decoded_instructions(number_of_accepted_decoded_instructions);
    decode_stage->number_of_leaving_instructions(number_of_decoded_instructions);
    for (int i=0 ; i<MAX_WIDTH ; i++) {
    	decode_stage->fetched_instruction[i](fetched_instruction[i]);
    	decode_stage->decoded_instruction[i](decoded_instruction[i]);
    }
  	
	execute_stage->clock(clock);
	execute_stage->number_of_accepted_decoded_instructions(number_of_accepted_decoded_instructions);
	execute_stage->number_of_decoded_instructions(number_of_decoded_instructions);
	for (int i=0 ; i<MAX_WIDTH ; i++) 
		execute_stage->decoded_instruction[i](decoded_instruction[i]);
	
	
 
  number_of_fetched_instructions.write(0);
  number_of_accepted_fetched_instructions.write(0);
  number_of_decoded_instructions.write(0);
  number_of_accepted_decoded_instructions.write(0);

	elm::cout << "Processor built!\n";
}

void ARMProcessor::step() {
	elm::cout << "ARMProcessor::step() begins\n";
	  clock.write(1);
      sc_start(0.5);
      clock.write(0);
      sc_start(0.5);
	elm::cout << "ARMProcessor::step() ends\n";
	
}

void FetchStage::fetch() {
	elm::cout << "fetch() begins\n";
	SimulatedInstruction* inst;
	int nb_fetched;
	int nb_sent;
	code_t code;
	
	elm::cout << "number_of_leaving_instructions=" << number_of_leaving_instructions.read() << "\n";
	elm::cout << "fetch queue size=" << fetch_queue->size() << "\n";
	for (int i=0 ; i<number_of_leaving_instructions.read() ; i++)
		fetch_queue->get();
	nb_fetched = 0;
	while ( (nb_fetched < width)
			&&
			(fetch_queue->size() != fetch_queue->capacity())) {
		if (next_inst->isConditional())
			next_inst = sim_state->driver->nextInstruction(true /* branch is taken */);
		else
			next_inst = sim_state->driver->nextInstruction();
		if (next_inst != NULL) {
			iss_fetch((address_t) (next_inst->address()),&code);
			inst = new SimulatedInstruction(next_inst,code);
			elm::cout << "fetching at " << next_inst->address() << "\n";
			fetch_queue->put(inst);
			nb_fetched++;
		}
	}
	nb_sent = MIN(width,fetch_queue->size());
	elm::cout << "nb fetched instructions sent=" << nb_sent << "\n";
	for (int i=0 ; i<nb_sent ; i++) 
		fetched_instruction[i] = fetch_queue->read(i);
	number_of_fetched_instructions = nb_sent;
	// recompute control signals
	control_request.write(!control_request.read());
	elm::cout << "fetch() ends\n";
}

void ExecuteStage::execute() {	
	elm::cout << "execute() begins\n";
	
	SimulatedInstruction* inst;
	for (int i=0 ; i<number_of_decoded_instructions.read() ; i++) {
		inst = (SimulatedInstruction *)decoded_instruction[i].read() ;
		elm::cout << "Executing instruction at pc=" << inst->inst()->address() << "\n";
		sim_state->driver->terminateInstruction(inst->inst());
		delete inst;
		
	}
	
	// recompute control signals
	control_request.write(!control_request.read());
	elm::cout << "execute() ends\n";
	
}
ARMProcessor::ARMProcessor(sc_module_name name) {
}

