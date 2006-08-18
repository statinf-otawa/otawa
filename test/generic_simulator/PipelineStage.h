#ifndef _PIPELINESTAGE_H_
#define _PIPELINESTAGE_H_

#include <systemc.h>
#include <elm/genstruct/SLList.h>
#include <InstructionQueue.h>
#include <SimulatedInstruction.h>
#include <elm/string/String.h>

typedef enum {	FETCH, 
				EXECUTE_IN_ORDER, 
				EXECUTE_OUT_OF_ORDER, 
				WRITEBACK, 
				COMMIT, 
				LAZYIQIQ} pipeline_stage_t;

class PipelineStageConfiguration {
	pipeline_stage_t stage_type;
	InstructionQueueConfiguration * input_queue;
	InstructionQueueConfiguration * output_queue;
	CString stage_name;
	int stage_width, in_stage_width, out_stage_width;
	public:
		PipelineStageConfiguration(CString name, pipeline_stage_t type,
			InstructionQueueConfiguration * inqueue, InstructionQueueConfiguration * outqueue,
			int in_width, int out_width);
		PipelineStageConfiguration(CString name, pipeline_stage_t type,
			InstructionQueueConfiguration * inqueue, InstructionQueueConfiguration * outqueue,
			int width);
		
		pipeline_stage_t type();
		CString name();
		InstructionQueueConfiguration * inputQueue();
		InstructionQueueConfiguration * outputQueue();
		int width();
		int inWidth();
		int outWidth();
};

class LazyStageIQIQ : public sc_module {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_in<SimulatedInstruction *> * in_instruction;
		sc_in<int> in_number_of_ins;
		sc_out<int> out_number_of_accepted_ins;
		sc_out<SimulatedInstruction *> * out_instruction;
		sc_out<int> out_number_of_outs;
		sc_in<int> in_number_of_accepted_outs;
		
	private:
		// variables
		int stage_width;
		
	public:
		LazyStageIQIQ(sc_module_name name, int width);
		
		SC_HAS_PROCESS(LazyStageIQIQ);
		void action();
};



#endif //_PIPELINESTAGE_H_


