#include <otawa/gensim/Execute.h>
#include <otawa/gensim/GenericSimulator.h>
#include <otawa/gensim/debug.h>

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
	TRACE(elm::cout << "ExecuteStage->action()\n";)
	TRACE(elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";)
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		inst = in_instruction[i].read() ;
		assert(inst->state() == READY);
		issued++;
		TRACE(elm::cout << "\texecuting at " << inst->inst()->address() << "\n";)
		inst->notifyResult(rename_tables);
		sim_state->driver->terminateInstruction(*sim_state, inst->inst());
//		iss_free(inst->emulatedInst());
		delete inst;
	}
	TRACE(elm::cout << "\tout_number_of_accepted_ins=" << in_number_of_ins.read() << "\n";)
	out_number_of_accepted_ins.write(in_number_of_ins.read());	
}

ExecuteOOOStage::ExecuteOOOStage(sc_module_name name, int width, 
						InstructionQueue * _rob,
						elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables)
		: PipelineStage(name) {
	stage_width = width;
	rob = _rob;
	rename_tables = _rename_tables;
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void ExecuteOOOStage::action() {
	int executed = 0;
	elm::genstruct::SLList<SimulatedInstruction *> executed_instructions;
	TRACE(elm::cout << "ExecuteStage->action()\n";)
	for (int i=0 ; ( (i<rob->size()) && (executed<stage_width) ) ; i++) {
		SimulatedInstruction * inst = rob->read(i);
		if (inst->state() == READY) {
			executed_instructions.addLast(inst);
			executed++;
		}
	}
	while (!executed_instructions.isEmpty()) {
		SimulatedInstruction * inst = executed_instructions.first();
		executed_instructions.removeFirst();
		inst->notifyResult(rename_tables);
		TRACE(elm::cout << "\texecuting at " << inst->inst()->address() << "\n";)
	}
}

CommitStage::CommitStage(sc_module_name name, int _width, otawa::GenericState * gen_state) 
		: PipelineStage(name) {
	width = _width;
	sim_state = gen_state;
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void CommitStage::action() {
	SimulatedInstruction* inst;
	TRACE(elm::cout << "CommitStage->action()\n";)
	TRACE(elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";)
	for (int i=0 ; i<in_number_of_ins.read() ; i++) {
		inst = in_instruction[i].read() ;
		assert(inst->state() == EXECUTED);
		TRACE(elm::cout << "\tcommitting at " << inst->inst()->address() << "\n";)
		sim_state->driver->terminateInstruction(*sim_state, inst->inst());
//		iss_free(inst->emulatedInst());
		delete inst;
	}
	TRACE(elm::cout << "\tout_number_of_accepted_ins=" << in_number_of_ins.read() << "\n";)
	out_number_of_accepted_ins.write(in_number_of_ins.read());	
}

