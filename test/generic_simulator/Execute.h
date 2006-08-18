#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include <systemc.h>
#include <SimulatedInstruction.h>

namespace otawa {
	class GenericState;
}


class ExecuteInOrderStageIQ : public sc_module {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_in<SimulatedInstruction *> * in_instruction;
		sc_in<int> in_number_of_ins;
		sc_out<int> out_number_of_accepted_ins;
		
	private:
		// variables
		int stage_width;
		otawa::GenericState * sim_state;
		
	public:
		ExecuteInOrderStageIQ(sc_module_name name, int width, otawa::GenericState * gen_state);
		
		SC_HAS_PROCESS(ExecuteInOrderStageIQ);
		void action();
};


#endif //_EXECUTE_H_
