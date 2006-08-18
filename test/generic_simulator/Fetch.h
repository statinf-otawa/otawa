#ifndef _FETCH_H_
#define _FETCH_H_

#include <PipelineStage.h>
#include <InstructionQueue.h>	
#include <SimulatedInstruction.h>
#include <emul.h>
#include <iss_include.h>
#include <otawa/otawa.h>

namespace otawa {
	class GenericState;
}

class FetchStage : public sc_module {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_out<SimulatedInstruction *> * out_fetched_instruction;
		sc_out<int> out_number_of_fetched_instructions;
		sc_in<int>in_number_of_accepted_instructions;
	
	private:	
		// parameters
		otawa::GenericState *sim_state;
		state_t * emulated_state;
		int out_ports;
		
		// variables
		otawa::Inst* next_inst;
		
	public:

		FetchStage(sc_module_name name, int number_of_out_ports, otawa::GenericState * gen_state);

		SC_HAS_PROCESS(FetchStage);	
		void fetch();

};

#endif //_FETCH_H_
