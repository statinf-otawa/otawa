#include <otawa/gensim/GenericProcessor.h>

GenericProcessor::GenericProcessor(sc_module_name name, ProcessorConfiguration * conf, otawa::GenericState * sim_state) {
	int iports,oports;
	InstructionQueue * input_queue;
	InstructionQueue * output_queue;
	bool found;
	sc_signal<int> * nb;
	
	
	for (elm::genstruct::SLList<InstructionQueueConfiguration *>::Iterator queue_conf(*(conf->instructionQueuesList())) ; 
			queue_conf ; queue_conf++) {
		InstructionQueue * new_queue = new InstructionQueue((sc_module_name) (queue_conf->name()), *queue_conf);
		instruction_queues.addLast(new_queue);
		new_queue->in_clock(clock);
	}
	
	for (elm::genstruct::SLList<PipelineStageConfiguration *>::Iterator stage_conf(*(conf->pipelineStagesList())) ; 
			stage_conf ; stage_conf++) {
		switch(stage_conf->type()) {
			case FETCH:	{		
				assert(stage_conf->outputQueue());
				found = false;
				for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(instruction_queues) ; iq ; iq++) {
					if (iq->configuration() == stage_conf->outputQueue()) {
						oports = iq->configuration()->numberOfWritePorts();
						output_queue = iq;
						found = true;
					}
				}
				assert(found);
				
				FetchStage * fetch_stage = new FetchStage((sc_module_name) (stage_conf->name()), oports, sim_state);	
				fetch_stage->in_clock(clock);
			
				
				for (int i=0 ; i<oports ; i++) {
					sc_signal<SimulatedInstruction *> * fetched_instruction = new sc_signal<SimulatedInstruction *>;
					fetch_stage->out_fetched_instruction[i](*fetched_instruction);
					output_queue->in_instruction[i](*fetched_instruction);
				}
				nb = new sc_signal<int>;
				nb->write(0);
				fetch_stage->out_number_of_fetched_instructions(*nb);
				output_queue->in_number_of_ins(*nb);
				nb = new sc_signal<int>;
				nb->write(output_queue->configuration()->numberOfWritePorts());
				fetch_stage->in_number_of_accepted_instructions(*nb);
				output_queue->out_number_of_accepted_ins(*nb);
			} break;
				
			case LAZYIQIQ: {
				assert(stage_conf->inputQueue());
				found = false;
				for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(instruction_queues) ; iq ; iq++) {
					if (iq->configuration() == stage_conf->inputQueue()) {
						iports = iq->configuration()->numberOfReadPorts();
						input_queue = iq;
						found = true;
					}
				}
				assert(found);
				assert(stage_conf->outputQueue());
				found = false;
				for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(instruction_queues) ; iq ; iq++) {
					if (iq->configuration() == stage_conf->outputQueue()) {
						oports = iq->configuration()->numberOfWritePorts();
						output_queue = iq;
						found = true;
					}
				}
				assert(found);
				
				LazyStageIQIQ * lazy_stage = new LazyStageIQIQ((sc_module_name) (stage_conf->name()), stage_conf->width());
				lazy_stage->in_clock(clock);
				
				for (int i=0 ; i<iports ; i++) {
					sc_signal<SimulatedInstruction *> * instruction = new sc_signal<SimulatedInstruction *>;
					lazy_stage->in_instruction[i](*instruction);
					input_queue->out_instruction[i](*instruction);
				}
								
				for (int i=0 ; i<oports ; i++) {
					sc_signal<SimulatedInstruction *> * instruction = new sc_signal<SimulatedInstruction *>;
					lazy_stage->out_instruction[i](*instruction);
					output_queue->in_instruction[i](*instruction);
				}
				nb = new sc_signal<int>;
				nb->write(0);
				lazy_stage->in_number_of_ins(*nb);
				input_queue->out_number_of_outs(*nb);
				nb = new sc_signal<int>;
				nb->write(0);
				lazy_stage->out_number_of_accepted_ins(*nb);
				input_queue->in_number_of_accepted_outs(*nb);
				nb = new sc_signal<int>;
				nb->write(0);
				lazy_stage->out_number_of_outs(*nb);
				output_queue->in_number_of_ins(*nb);
				nb = new sc_signal<int>;
				nb->write(output_queue->configuration()->numberOfWritePorts());
				lazy_stage->in_number_of_accepted_outs(*nb);
				output_queue->out_number_of_accepted_ins(*nb);
			} break;
				
			case EXECUTE_IN_ORDER: {
				assert(stage_conf->inputQueue());
				found = false;
				for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(instruction_queues) ; iq ; iq++) {
					if (iq->configuration() == stage_conf->inputQueue()) {
						iports = iq->configuration()->numberOfReadPorts();
						input_queue = iq;
						found = true;
					}
				}
				assert(found);
				ExecuteInOrderStageIQ * execute_stage = 
					new ExecuteInOrderStageIQ((sc_module_name) (stage_conf->name()), iports, sim_state);	
				execute_stage->in_clock(clock);
			
				
				for (int i=0 ; i<iports ; i++) {
					sc_signal<SimulatedInstruction *> * instruction = new sc_signal<SimulatedInstruction *>;
					execute_stage->in_instruction[i](*instruction);
					input_queue->out_instruction[i](*instruction);
				}
				nb = new sc_signal<int>;
				nb->write(0);
				execute_stage->in_number_of_ins(*nb);
				input_queue->out_number_of_outs(*nb);
				nb = new sc_signal<int>;
				nb->write(0);
				execute_stage->out_number_of_accepted_ins(*nb);
				input_queue->in_number_of_accepted_outs(*nb);
			} break;
				
			default:
				break;
		}
	}
	clock.write(0);
	
}

bool GenericProcessor::isEmpty() {
	bool empty = true;
	elm::cout << "Is processor empty ??\n";
	for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(instruction_queues) ; iq ; iq++) {
		empty = empty && iq->isEmpty();
		elm::cout  << "\t" << iq->name() << ":" << iq->isEmpty() << "\n";
	}
	elm::cout << "\tconclusion: " << empty << "\n";
	return empty;
}

void GenericProcessor::step() {
	elm::cout << "----- GenericProcessor->Step() : rising edge \n";
	clock.write(1);
	sc_start(0.5);
	elm::cout << "----- GenericProcessor->Step() : falling edge \n";
	clock.write(0);
	sc_start(0.5);
}


