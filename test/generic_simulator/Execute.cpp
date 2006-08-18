#include <Execute.h>
#include <GenericSimulator.h>

ExecuteInOrderStageIQ::ExecuteInOrderStageIQ(sc_module_name name, int width, otawa::GenericState * gen_state) {
	stage_width = width;
	sim_state = gen_state;
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	
	SC_METHOD(action);
	sensitive_neg << in_clock;
}

void ExecuteInOrderStageIQ::action() {
	SimulatedInstruction* inst;
	int issued = 0;
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		inst = in_instruction[i].read() ;
		issued++;
		sim_state->driver->terminateInstruction(*sim_state, inst->inst());
//		iss_free(inst->emulatedInst());
		delete inst;
	}
	
	out_number_of_accepted_ins.write(stage_width);	
}
