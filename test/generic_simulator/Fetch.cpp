#include <Fetch.h>
#include <GenericSimulator.h>

FetchStage::FetchStage(sc_module_name name, int number_of_out_ports, otawa::GenericState * gen_state) {
	out_fetched_instruction = new sc_out<SimulatedInstruction *>[number_of_out_ports];
	out_ports = number_of_out_ports;
	sim_state = gen_state;
	SC_METHOD(fetch);
	sensitive_pos << in_clock;
}

void FetchStage::fetch() {
	
	SimulatedInstruction* inst;
	int nb_fetched;
	code_t code;
	instruction_t *emulated_inst;
	
	elm::cout << "Fetchstage->fetch():\n";
	elm::cout << "\tin_number_of_accepted_instructions=" << in_number_of_accepted_instructions.read() << "\n";
	
	nb_fetched = 0;
	do  {
		next_inst = sim_state->driver->nextInstruction(*sim_state, next_inst);
		if (next_inst != NULL) {
			elm::cout << "fetching at " << next_inst->address() << "\n";
//			iss_fetch((::address_t)(unsigned long)next_inst->address(), &code);
//			emulated_inst = iss_decode((::address_t)(unsigned long)next_inst->address(), &code);
//			iss_complete(emulated_inst,emulated_state);
			inst = new SimulatedInstruction(next_inst,code, emulated_inst);
			out_fetched_instruction[nb_fetched] = inst;
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
	while  ( (nb_fetched < in_number_of_accepted_instructions.read())
			&&
			(next_inst != NULL) );
	out_number_of_fetched_instructions.write(nb_fetched);
	elm::cout << "\tout_number_of_fetched_instructions=" << nb_fetched << "\n";
}
