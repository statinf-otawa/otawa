#ifndef _SIMULATEDINSTRUCTION_H_
#define _SIMULATEDINSTRUCTION_H_

#include <otawa/otawa.h>
#include <otawa/hard/Register.h>
#include <elm/genstruct/Table.h>
#include <emul.h>
#include <otawa/gensim/debug.h>

typedef enum {NONE, WAITING, READY, EXECUTING, EXECUTED, NOTIFIED} simulated_instruction_state_t; // ordered set

typedef enum {COND_BRANCH, UNCOND_BRANCH, TRAP, CALL, RETURN, LOAD, STORE, IALU, FALU, MUL, DIV, OTHER} instruction_type_t;
#define INST_TYPE_NUMBER 12
	// FIXME : should be read from framework


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
		int time_to_finish_execution;
		instruction_type_t _type;
		
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
		inline int timeToFinish();
		inline void setTimeToFinish(int time);
		inline int decrementTimeToFinish();
		inline void dump(elm::io::Output& out_stream);
		inline void dumpState(elm::io::Output& out_stream);
		inline void dumpType(elm::io::Output& out_stream);
		instruction_type_t type();
		
};

inline SimulatedInstruction::SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst,
												elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions) :
		instruction(inst), binary_code(code), emulated_inst(emul_inst), instruction_state(READY), 
		active_instructions(_active_instructions) {
	active_instructions->addLast(this);
	if (inst->kind() & otawa::Inst::IS_CONTROL) {
		if (inst->kind() & otawa::Inst::IS_CALL)
			_type = CALL;
		else if (inst->kind() & otawa::Inst::IS_RETURN)
			_type = RETURN;
		else if (inst->kind() & otawa::Inst::IS_TRAP)
			_type = TRAP;
		else if (inst->kind() & otawa::Inst::IS_COND)
			_type = COND_BRANCH;
		else
			_type = UNCOND_BRANCH;
	}
	else if (inst->kind() & otawa::Inst::IS_LOAD)
		_type = LOAD;
	else if (inst->kind() & otawa::Inst::IS_STORE)
		_type = STORE;
	else if (inst->kind() & otawa::Inst::IS_MUL)
		_type = MUL;
	else if (inst->kind() & otawa::Inst::IS_DIV)
		_type = DIV;
	else if (inst->kind() & otawa::Inst::IS_INT)
		_type = IALU;
	else if (inst->kind() & otawa::Inst::IS_FLOAT)
		_type = FALU;
	else _type = OTHER;
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

inline instruction_type_t SimulatedInstruction::type() {
	return _type;
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
		if (inst->state() == WAITING) {
			inst->sourceInstructions()->remove(this);
			if (inst->sourceInstructions()->isEmpty()) {
				inst->setState(READY);
				TRACE(elm::cout << "\t\tinst " << inst->inst()->address() << " is ready now \n");
			}
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

inline int SimulatedInstruction::timeToFinish() {
	return time_to_finish_execution;
}

inline void SimulatedInstruction::setTimeToFinish(int time) {
	time_to_finish_execution = time;
}

inline int SimulatedInstruction::decrementTimeToFinish() {
	time_to_finish_execution--;
	assert(time_to_finish_execution >= 0);
	return time_to_finish_execution;
}


inline void SimulatedInstruction::dump(elm::io::Output& out_stream) {
	out_stream << inst()->address() << ": " ;
	inst()->dump(out_stream);
	out_stream << " [";
	dumpType(out_stream);
	out_stream << "] - ";
	dumpState(out_stream);
	if (instruction_state == WAITING) {
		out_stream << " - waits for ";
		for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator inst(source_instructions) ; inst ; inst++) {
			out_stream << inst->inst()->address() << ", ";
		}
	}
	out_stream << "\n";
}

inline void SimulatedInstruction::dumpState(elm::io::Output& out_stream) {
	switch(instruction_state) {
		case NONE:
			out_stream << "NONE";
			break;
		case WAITING:
			out_stream << "WAITING";
			break;
		case READY:
			out_stream << "READY";
			break;
		case EXECUTING:
			out_stream << "EXECUTING";
			break;
		case EXECUTED:
			out_stream << "EXECUTED";
			break;
		case NOTIFIED:
			out_stream << "NOTIFIED";
			break;
	}
}

inline void SimulatedInstruction::dumpType(elm::io::Output& out_stream) {
	switch(_type) {
		case COND_BRANCH:
			out_stream << "COND_BRANCH";
			break;
		case UNCOND_BRANCH:
			out_stream << "UNCOND_BRANCH";
			break;
		case TRAP:
			out_stream << "TRAP";
			break;
		case CALL:
			out_stream << "CALL";
			break;
		case RETURN:
			out_stream << "RETURN";
			break;
		case LOAD:
			out_stream << "LOAD";
			break;
		case STORE:
			out_stream << "STORE";
			break;
		case IALU:
			out_stream << "IALU";
			break;
		case FALU:
			out_stream << "FALU";
			break;
		case MUL:
			out_stream << "MUL";
			break;
		case DIV:
			out_stream << "DIV";
			break;
		case OTHER:
			out_stream << "OTHER";
			break;
	}
}


#endif //_SIMULATEDINSTRUCTION_H_
