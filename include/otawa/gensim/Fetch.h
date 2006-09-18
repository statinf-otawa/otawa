#ifndef _FETCH_H_
#define _FETCH_H_

#include <otawa/gensim/PipelineStage.h>
#include <otawa/gensim/InstructionQueue.h>	
#include <otawa/gensim/SimulatedInstruction.h>
//#include <emul.h>
//#include <iss_include.h>
#include <otawa/otawa.h>

namespace otawa {
	class GenericState;
}

#define NB_INT_REGS 32
#define NB_FP_REGS 32
	// FIXME: should be learned from gliss

class FetchStage : public PipelineStage {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_out<SimulatedInstruction *> * out_fetched_instruction;
		sc_out<int> out_number_of_fetched_instructions;
		sc_in<int>in_number_of_accepted_instructions;
	
	private:	
		// parameters
		otawa::GenericState *sim_state;
		//state_t * emulated_state;
		int out_ports;
		elm::genstruct::SLList<SimulatedInstruction *> fetched_instructions;
		elm::genstruct::SLList<SimulatedInstruction *> * active_instructions;
		
		// variables
		otawa::Inst* next_inst;
		elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
		
	public:

		FetchStage(sc_module_name name, int number_of_out_ports, otawa::GenericState * gen_state,
					elm::genstruct::AllocatedTable<rename_table_t> * rename_tables,
					elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions);

		SC_HAS_PROCESS(FetchStage);	
		void fetch();

};

#endif //_FETCH_H_
