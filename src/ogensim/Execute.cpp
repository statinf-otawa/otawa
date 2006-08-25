#include <otawa/gensim/Execute.h>
#include <otawa/gensim/GenericSimulator.h>

ExecuteInOrderStageIQ::ExecuteInOrderStageIQ(sc_module_name name, int width, otawa::GenericState * gen_state,
										elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables) 
		: PipelineStage(name) {
	stage_width = width;
	sim_state = gen_state;
	rename_tables = _rename_tables;
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void ExecuteInOrderStageIQ::action() {
	SimulatedInstruction* inst;
	int issued = 0;
	elm::cout << "ExecuteStage->action()\n";
	elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		inst = in_instruction[i].read() ;
		assert(inst->state() == READY);
		issued++;
		elm::cout << "\texecuting at " << inst->inst()->address() << "\n";
		inst->notifyResult(rename_tables);
		sim_state->driver->terminateInstruction(*sim_state, inst->inst());
//		iss_free(inst->emulatedInst());
		delete inst;
	}
	elm::cout << "\tout_number_of_accepted_ins=" << in_number_of_ins.read() << "\n";
	out_number_of_accepted_ins.write(in_number_of_ins.read());	
}

bool ExecuteInOrderStageIQ::isEmpty() {
	return true;
}
