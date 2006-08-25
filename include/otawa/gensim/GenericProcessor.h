#ifndef _GENERICPROCESSOR_H_
#define _GENERICPROCESSOR_H_

#include <elm/genstruct/SLList.h>
#include <systemc.h>
#include <otawa/gensim/PipelineStage.h>
#include <otawa/gensim/Fetch.h>
#include <otawa/gensim/Execute.h>
#include <otawa/gensim/SimulatedInstruction.h>

namespace otawa {
	class GenericState;
}


class ProcessorConfiguration {
	elm::genstruct::SLList<PipelineStageConfiguration *> pipeline_stages;
	elm::genstruct::SLList<InstructionQueueConfiguration *> instruction_queues;
	public:
		void addPipelineStage(PipelineStageConfiguration * new_stage) {
			pipeline_stages.addLast(new_stage);
		}
		void addInstructionQueue(InstructionQueueConfiguration * new_queue) {
			instruction_queues.addLast(new_queue);
		}
		elm::genstruct::SLList<InstructionQueueConfiguration *> * instructionQueuesList() {
			return &instruction_queues;
		}
		elm::genstruct::SLList<PipelineStageConfiguration *> * pipelineStagesList() {
			return &pipeline_stages;
		}
		
};

SC_MODULE(GenericProcessor)
{
	// signals
	sc_signal<bool> clock;
	
	// variables
	elm::genstruct::SLList<InstructionQueue *> instruction_queues;
	elm::genstruct::SLList<PipelineStage *> pipeline_stages;
	elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
	elm::genstruct::SLList<SimulatedInstruction *> active_instructions;
	public:
		bool isEmpty(); 
		void step(); 
		void Flush();
	
  	GenericProcessor(sc_module_name name, ProcessorConfiguration * conf, otawa::GenericState * sim_state, otawa::hard::Platform *pf); 
  	~GenericProcessor() {
   	}
};

#endif //_GENERICPROCESSOR_H_

