#include <otawa/gensim/GenericProcessor.h>
#include <otawa/gensim/debug.h>

namespace otawa { namespace gensim {

void ProcessorConfiguration::dump(elm::io::Output& out_stream) {
	out_stream << "---- Processor configuration ----\n";
	out_stream << " Instruction queues:\n";
	for (elm::genstruct::SLList<InstructionQueueConfiguration *>::Iterator iqc(instruction_queues) ; iqc ; iqc++) {
		iqc->dump(out_stream);
	}
	out_stream << " Pipeline stages:\n";
	for (elm::genstruct::SLList<PipelineStageConfiguration *>::Iterator psc(pipeline_stages) ; psc ; psc++) {
		psc->dump(out_stream);
	}
	out_stream << "---- end of configuration ----\n";
	
}


GenericProcessor::GenericProcessor(sc_module_name name, ProcessorConfiguration * conf, 
									GenericState * sim_state, otawa::hard::Platform *pf) {
	int iports,oports;
	InstructionQueue * input_queue;
	InstructionQueue * output_queue;
	bool found;
	sc_signal<int> * nb;
	
	// Init rename tables
	rename_tables = new elm::genstruct::AllocatedTable<rename_table_t>(pf->banks().count());
	int reg_bank_count = pf->banks().count();
	for(int i = 0; i <reg_bank_count ; i++) {
		(*rename_tables)[i].reg_bank = (otawa::hard::RegBank *) pf->banks()[i];
		(*rename_tables)[i].table = 
			new elm::genstruct::AllocatedTable<SimulatedInstruction *>((*rename_tables)[i].reg_bank->count());
		for (int j=0 ; j<(*rename_tables)[i].reg_bank->count() ; j++)
			(*rename_tables)[i].table->set(j,NULL);
	}
	
	
	
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
				
				FetchStage * fetch_stage = new FetchStage((sc_module_name) (stage_conf->name()), 
															oports, sim_state, rename_tables, &active_instructions);	
				pipeline_stages.addLast(fetch_stage);
				
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
				nb->write(0);
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
				pipeline_stages.addLast(lazy_stage);
				
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
				nb->write(0);
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
					new ExecuteInOrderStageIQ((sc_module_name) (stage_conf->name()), iports, sim_state, 
											rename_tables, conf->functionalUnitsList());	
				pipeline_stages.addLast(execute_stage);
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
			
			case EXECUTE_OUT_OF_ORDER: {
				found = false;
				InstructionQueue * rob;
				for (elm::genstruct::SLList<InstructionQueue *>::Iterator iq(instruction_queues) ; iq ; iq++) {
					if (iq->configuration() == stage_conf->instructionBuffer()) {
						rob = iq;
						found = true;
					}
				}
				assert(found);
				ExecuteOOOStage * execute_stage = 
					new ExecuteOOOStage((sc_module_name) (stage_conf->name()), (stage_conf->width()), rob, 
										rename_tables, conf->functionalUnitsList());	
				pipeline_stages.addLast(execute_stage);
				execute_stage->in_clock(clock);
			} break;
		
			case COMMIT: {
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
				CommitStage * commit_stage = 
					new CommitStage((sc_module_name) (stage_conf->name()), iports, sim_state);	
				pipeline_stages.addLast(commit_stage);
				commit_stage->in_clock(clock);
			
				
				for (int i=0 ; i<iports ; i++) {
					sc_signal<SimulatedInstruction *> * instruction = new sc_signal<SimulatedInstruction *>;
					commit_stage->in_instruction[i](*instruction);
					input_queue->out_instruction[i](*instruction);
				}
				nb = new sc_signal<int>;
				nb->write(0);
				commit_stage->in_number_of_ins(*nb);
				input_queue->out_number_of_outs(*nb);
				nb = new sc_signal<int>;
				nb->write(0);
				commit_stage->out_number_of_accepted_ins(*nb);
				input_queue->in_number_of_accepted_outs(*nb);
			} break;
			
				
			default:
				break;
		}
	}
	clock.write(0);
//	TRACE(dump(elm::cout);)
}

bool GenericProcessor::isEmpty() {
	return (active_instructions.count() == 0);
}

void GenericProcessor::step() {
	TRACE(elm::cout << "----- GenericProcessor->Step() : rising edge \n";)
	clock.write(1);
	sc_start(0.5);
	TRACE(elm::cout << "----- GenericProcessor->Step() : falling edge \n";)
	clock.write(0);
	sc_start(0.5);
}

void GenericProcessor::Flush() {
	for(int i = 0; i <rename_tables->count() ; i++) {
		for (int j=0 ; j<(*rename_tables)[i].reg_bank->count() ; j++)
			(*rename_tables)[i].table->set(j,NULL);
	}
	
}

} } // otawa::gensim
