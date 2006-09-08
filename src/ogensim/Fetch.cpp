#include <otawa/gensim/Fetch.h>
#include <otawa/gensim/GenericState.h>
#include <otawa/gensim/debug.h>

FetchStage::FetchStage(sc_module_name name, int number_of_out_ports, otawa::GenericState * gen_state,
	elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
	elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions) 
	: PipelineStage(name) {
	out_fetched_instruction = new sc_out<SimulatedInstruction *>[number_of_out_ports];
	out_ports = number_of_out_ports;
	sim_state = gen_state;
	rename_tables = _rename_tables;
	active_instructions = _active_instructions;
	SC_METHOD(fetch);
	sensitive_pos << in_clock;
}

void FetchStage::fetch() {
	
	SimulatedInstruction* inst;
	int nb_fetched;
	code_t code;
	instruction_t *emulated_inst;
	
	TRACE(elm::cout << "Fetchstage->fetch():\n";)
	TRACE(elm::cout << "\tin_number_of_accepted_instructions=" << in_number_of_accepted_instructions.read() << "\n";)
	for (int i=0 ; i<in_number_of_accepted_instructions.read() ; i++)
		fetched_instructions.removeFirst();
	nb_fetched = fetched_instructions.count();
	do  {
		next_inst = sim_state->driver->nextInstruction(*sim_state, next_inst);
		if (next_inst != NULL) {
			TRACE(elm::cout << "\tfetching at " << next_inst->address() << "\n";)
//			iss_fetch((::address_t)(unsigned long)next_inst->address(), &code);
//			emulated_inst = iss_decode((::address_t)(unsigned long)next_inst->address(), &code);
//			iss_complete(emulated_inst,emulated_state);
			inst = new SimulatedInstruction(next_inst,code, emulated_inst, active_instructions);
			inst->renameOperands(rename_tables);
			//out_fetched_instruction[nb_fetched] = inst;
			fetched_instructions.addLast(inst);
			nb_fetched++;
//			if (next_inst->isConditional()) {
//				if (NIA(emulated_state) == CIA(emulated_state) + sizeof(code_t))
//					next_inst = sim_state->driver->nextInstruction(false /* branch is not taken */);
//				else
//					next_inst = sim_state->driver->nextInstruction(true /* branch is taken */);
//			}
//			else
//				next_inst = sim_state->driver->nextInstruction(*sim_state, next_inst);
		}
		
	}
	while  ( (nb_fetched < out_ports)
			&&
			(next_inst != NULL) );
	int outs = 0;
	for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator inst(fetched_instructions) ; inst ; inst++) {
		if (outs<out_ports) {// FIXME 
			out_fetched_instruction[outs++] = inst;
		}
	}
	out_number_of_fetched_instructions.write(outs);
	TRACE(elm::cout << "\tout_number_of_fetched_instructions=" << nb_fetched << "\n";)
}
