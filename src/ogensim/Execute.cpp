#include <otawa/gensim/Execute.h>
#include <otawa/gensim/GenericState.h>
#include <otawa/gensim/debug.h>

FunctionalUnitConfiguration::FunctionalUnitConfiguration(bool is_pipelined, int latency, int width)
	: _is_pipelined(is_pipelined), _latency(latency), _width(width) {
	if (is_pipelined)
		assert(latency > 1);
}

bool FunctionalUnitConfiguration::isPipelined() {
	return _is_pipelined;
}

int FunctionalUnitConfiguration::latency() {
	return _latency;
}

int FunctionalUnitConfiguration::width() {
	return _width;
}

void FunctionalUnitConfiguration::addInstructionType(instruction_type_t type) {
	instruction_types.addLast(type);
}

elm::genstruct::SLList<instruction_type_t> * FunctionalUnitConfiguration::instructionTypes() {
	return &instruction_types;
}

FunctionalUnit::FunctionalUnit(bool is_pipelined, int latency, int width)
	: _is_pipelined(is_pipelined), _latency(latency), _width(width), new_instructions(0), pending_instructions(0) {
}

bool FunctionalUnit::isPipelined() {
	return _is_pipelined;
}

int FunctionalUnit::latency() {
	return _latency;
}

int FunctionalUnit::width() {
	return _width;
}



ExecuteInOrderStageIQ::ExecuteInOrderStageIQ(sc_module_name name, int width, otawa::GenericState * gen_state,
										elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
										elm::genstruct::SLList<FunctionalUnitConfiguration *> * _functional_units) 
		: PipelineStage(name) {
	stage_width = width;
	sim_state = gen_state;
	rename_tables = _rename_tables;
	in_instruction = new sc_in<SimulatedInstruction *>[width];
	int fu_number = _functional_units->count();
	functional_units = new elm::genstruct::AllocatedTable<FunctionalUnit *>(fu_number);
	int inst_type_number = INST_TYPE_NUMBER /*FIXME*/;
	fu_bindings = new elm::genstruct::AllocatedTable<FunctionalUnit *>(inst_type_number);
	for (int index=0 ; index<inst_type_number ; index++) {
		(*fu_bindings)[index] = NULL;
	}
	number_of_functional_units = 0;
	for (elm::genstruct::SLList<FunctionalUnitConfiguration *>::Iterator fu_conf(*_functional_units) ; fu_conf ; fu_conf++) {
		FunctionalUnit * fu = new FunctionalUnit(fu_conf->isPipelined(), fu_conf->latency(), fu_conf->width());
		(*functional_units)[number_of_functional_units++] = fu;
		
		for (elm::genstruct::SLList<instruction_type_t>::Iterator type(*(fu_conf->instructionTypes())) ; type ; type++) {
			assert((*fu_bindings)[*type] == NULL); // error: instruction type handled by two different functional units
			(*fu_bindings)[*type] = fu;
		}
	}
	
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void ExecuteInOrderStageIQ::action() {
	SimulatedInstruction* inst;
	int issued = 0;
	TRACE(elm::cout << "ExecuteStage->action()\n";)
	TRACE(elm::cout << "\tin_number_of_ins=" << in_number_of_ins.read() << "\n";)
	bool ok = true;
	while ( ok && (issued < in_number_of_ins.read()) ){
		inst = in_instruction[issued].read() ;
		assert(inst->state() == READY);
		instruction_type_t type;
		FunctionalUnit * fu = (*fu_bindings)[inst->type()];
		assert(fu);
		TRACE(elm::cout << "\tready instruction: " << inst->inst()->address();
			elm::cout << " [FU pipelined=" << fu->isPipelined() << " - width=" << fu->width();
			elm::cout << " - new=" << fu->newInstructions() << " - pending=" << fu->pendingInstructions() << "]\n";)
		if (  ( fu->isPipelined()
				&& 
				(fu->newInstructions() < fu->width()) )
			|| 
				( !fu->isPipelined()) 
				&& 
				(fu->pendingInstructions() < fu->width()) ) {
			fu->addInstruction();
			inst->setState(EXECUTING);
			inst->setTimeToFinish(fu->latency());
			executing_instructions.addLast(inst);
			TRACE(elm::cout << "\tstarting execution at " << inst->inst()->address() << " (duration=" << fu->latency() << ")\n";)	
			issued++;
		}
		else
			ok = false;
		
	}
	elm::genstruct::SLList<SimulatedInstruction *> terminated_instructions;
	for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator exec(executing_instructions) ; exec ; exec++) {
		if (exec->decrementTimeToFinish() == 0) {
			terminated_instructions.addLast(exec);
		}
	}
	
	while (!terminated_instructions.isEmpty()) {
		inst = terminated_instructions.first();
		inst->notifyResult(rename_tables);
		sim_state->driver->terminateInstruction(*sim_state, inst->inst());
		FunctionalUnit * fu = (*fu_bindings)[inst->type()];
		fu->subInstruction();
		executing_instructions.remove(inst);
		terminated_instructions.removeFirst();
		TRACE(elm::cout << "\tterminating execution at " << inst->inst()->address() << " \n";)	
		//		iss_free(inst->emulatedInst());
		delete inst;
	}
	TRACE(elm::cout << "\tout_number_of_accepted_ins=" << in_number_of_ins.read() << "\n";)
	out_number_of_accepted_ins.write(issued);	
	
	for (int i=0 ; i<number_of_functional_units ; i++) {
		(*functional_units)[i]->resetNewInstructions();
	}
	
}



ExecuteOOOStage::ExecuteOOOStage(sc_module_name name, int width, 
						InstructionQueue * _rob,
						elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
						elm::genstruct::SLList<FunctionalUnitConfiguration *> * _functional_units)
		: PipelineStage(name) {
	stage_width = width;
	rob = _rob;
	rename_tables = _rename_tables;
	int fu_number = _functional_units->count();
	functional_units = new elm::genstruct::AllocatedTable<FunctionalUnit *>(fu_number);
	int inst_type_number = INST_TYPE_NUMBER /*FIXME*/;
	fu_bindings = new elm::genstruct::AllocatedTable<FunctionalUnit *>(inst_type_number);
	for (int index=0 ; index<inst_type_number ; index++) {
		(*fu_bindings)[index] = NULL;
	}
	number_of_functional_units = 0;
	for (elm::genstruct::SLList<FunctionalUnitConfiguration *>::Iterator fu_conf(*_functional_units) ; fu_conf ; fu_conf++) {
		FunctionalUnit * fu = new FunctionalUnit(fu_conf->isPipelined(), fu_conf->latency(), fu_conf->width());
		(*functional_units)[number_of_functional_units++] = fu;
		
		for (elm::genstruct::SLList<instruction_type_t>::Iterator type(*(fu_conf->instructionTypes())) ; type ; type++) {
			assert((*fu_bindings)[*type] == NULL); // error: instruction type handled by two different functional units
			(*fu_bindings)[*type] = fu;
		}
	}
	SC_METHOD(action);
	sensitive_pos << in_clock;
}

void ExecuteOOOStage::action() {
	int executed = 0;
	TRACE(elm::cout << "ExecuteStage->action()\n";)
	bool memory_pending = false, memory_ordering = false;
	int i;
	for (i=0 ; ( (i<rob->size()) && (executed<stage_width) && !memory_ordering) ; i++) {
		SimulatedInstruction * inst = rob->read(i);
		if (inst->inst()->isMem()) {
			if (memory_pending) {
				memory_ordering = true;
				continue;
			}
			else
				memory_pending = true;
		}
		if (inst->state() == READY) {
			FunctionalUnit * fu = (*fu_bindings)[inst->type()];
			#ifndef NDEBUG
				if(!fu) {
					elm::cerr << "No function unit for " << inst->type() << " instruction type.\n";
					assert(false);
				}
			#endif
			TRACE(elm::cout << "\tready instruction: " << inst->inst()->address();
				elm::cout << " [FU pipelined=" << fu->isPipelined() << " - width=" << fu->width();
				elm::cout << " - new=" << fu->newInstructions() << " - pending=" << fu->pendingInstructions() << "]\n";)	
			if (  ( fu->isPipelined()
					&& 
					(fu->newInstructions() < fu->width()) )
				|| 
				  ( !fu->isPipelined()) 
				  	&& 
				  	(fu->pendingInstructions() < fu->width()) ) {
				inst->setState(EXECUTING);
				inst->setTimeToFinish(fu->latency());
				fu->addInstruction();
				executed++;
				TRACE(elm::cout << "\texecuting at " << inst->inst()->address() << "\n";)	
			}
		}
	}
	TRACE(elm::cout << "\tend because:";
			if (i==rob->size())
				elm::cout << "i==robsize - ";
			if (executed >= stage_width)
				elm::cout << "executed=" << executed << " - ";
			if (memory_ordering)
				elm::cout << "memory ordering";
			elm::cout << "\n";)
	for (int i=0 ; i<rob->size() ; i++) {
		SimulatedInstruction * inst = rob->read(i);
		if ( inst->state() == EXECUTING) {
			if (inst->decrementTimeToFinish() == 0) {
				inst->notifyResult(rename_tables);
				TRACE(elm::cout << "\tterminating execution at " << inst->inst()->address() << "\n";)
				(*fu_bindings)[inst->type()]->subInstruction();
			}
		}
	}
	for (int i=0 ; i<number_of_functional_units ; i++) {
		(*functional_units)[i]->resetNewInstructions();
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

