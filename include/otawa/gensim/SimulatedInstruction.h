#ifndef _SIMULATEDINSTRUCTION_H_
#define _SIMULATEDINSTRUCTION_H_

#include <otawa/otawa.h>
#include <otawa/hard/Register.h>
#include <elm/genstruct/Table.h>

#include <emul.h>

typedef enum {NONE, WAITING, READY, EXECUTED, TERMINATED} simulated_instruction_state_t; // ordered set

class SimulatedInstruction;

typedef struct rename_table_t {
	otawa::hard::RegBank * reg_bank;
	elm::genstruct::AllocatedTable<SimulatedInstruction *> *table;
} rename_table_t;

class SimulatedInstruction {
	private:
		elm::genstruct::SLList<SimulatedInstruction *> * active_instructions;
		otawa::Inst * instruction;
		code_t binary_code;
		instruction_t * emulated_inst;
		simulated_instruction_state_t instruction_state;
		elm::genstruct::SLList<SimulatedInstruction *> source_instructions;
		
		inline void addSourceInstruction(SimulatedInstruction * source_inst);
		inline void removeSourceInstruction(SimulatedInstruction * source_inst);
		
	public:
		inline SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst,
									elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions);
		inline ~SimulatedInstruction();
		inline otawa::Inst * inst();
		inline instruction_t * emulatedInst();
		inline void setState(simulated_instruction_state_t new_state);
		inline simulated_instruction_state_t state();
		inline elm::genstruct::SLList<SimulatedInstruction *> * sourceInstructions();
		inline void renameOperands(elm::genstruct::AllocatedTable<rename_table_t> * rename_tables);
		inline void notifyResult(elm::genstruct::AllocatedTable<rename_table_t> * rename_tables);
		
};

inline SimulatedInstruction::SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst,
												elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions) :
	instruction(inst), binary_code(code), emulated_inst(emul_inst), instruction_state(READY), active_instructions(_active_instructions) {
		active_instructions->addLast(this);
}

inline SimulatedInstruction::~SimulatedInstruction() {
	active_instructions->remove(this);
}
	
inline otawa::Inst * SimulatedInstruction::inst() {
	return instruction;
}

inline instruction_t * SimulatedInstruction::emulatedInst() {
	return emulated_inst;
}

inline void SimulatedInstruction::setState(simulated_instruction_state_t new_state) {
	instruction_state = new_state;
}

inline simulated_instruction_state_t SimulatedInstruction::state() {
	return instruction_state;
}

inline void SimulatedInstruction::addSourceInstruction(SimulatedInstruction * source_inst) {
	source_instructions.addLast(source_inst);
}

inline void SimulatedInstruction::removeSourceInstruction(SimulatedInstruction * source_inst) {
	source_instructions.remove(source_inst);
}


inline elm::genstruct::SLList<SimulatedInstruction *> * SimulatedInstruction::sourceInstructions() {
	return &source_instructions;
}

inline void SimulatedInstruction::renameOperands(elm::genstruct::AllocatedTable<rename_table_t> * rename_tables) {
	const elm::genstruct::Table<otawa::hard::Register *> &reads = 
			this->inst()->readRegs();
	for(int i = 0; i < reads.count(); i++) {
		for (int b=0 ; b<rename_tables->count() ; b++) {
			if ((*rename_tables)[b].reg_bank == reads[i]->bank()) {
				SimulatedInstruction * producing_inst = (*rename_tables)[b].table->get(reads[i]->number());
				if (producing_inst != NULL) {
					this->removeSourceInstruction(producing_inst); // FIXME
					this->addSourceInstruction(producing_inst);
					this->setState(WAITING);
					elm::cout << "\n";
				}				
			}
		}
	}
	const elm::genstruct::Table<otawa::hard::Register *>& writes = 
		this->inst()->writtenRegs();
	for(int i = 0; i < writes.count(); i++) {
		for (int b=0 ; b<rename_tables->count() ; b++) {
			if ((*rename_tables)[b].reg_bank == writes[i]->bank()) {
				(*rename_tables)[b].table->set(writes[i]->number(),this);
			}
		}
	}
	
}

inline void SimulatedInstruction::notifyResult(elm::genstruct::AllocatedTable<rename_table_t> * rename_tables) {
	for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator inst(*active_instructions) ; inst ; inst++) {
		inst->sourceInstructions()->remove(this);
		elm::cout << "\tnotifying inst " << inst->inst()->address() << "\n";
		if (inst->sourceInstructions()->isEmpty()) {
			inst->setState(READY);
			elm::cout << "\t\tinst " << inst->inst()->address() << " is ready now \n";
		}
		else {
			elm::cout << "\t\tinst " << inst->inst()->address() << "waits for";
			for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator i(*(inst->sourceInstructions())) ; i ; i++) {
				elm::cout << i->inst()->address() << " - ";
			}
			elm::cout << " \n";
		}
	}
	this->setState(EXECUTED);
	const elm::genstruct::Table<otawa::hard::Register *>& writes = 
		this->inst()->writtenRegs();
	for(int i = 0; i < writes.count(); i++) {
		for (int b=0 ; b<rename_tables->count() ; b++) {
			if ((*rename_tables)[b].reg_bank == writes[i]->bank()) {
				if ((*rename_tables)[b].table->get(writes[i]->number()) == this )
					(*rename_tables)[b].table->set(writes[i]->number(),NULL);
			}
		}
	}
	
}



#endif //_SIMULATEDINSTRUCTION_H_
