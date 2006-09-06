#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include <systemc.h>
#include <otawa/gensim/SimulatedInstruction.h>
#include <otawa/gensim/PipelineStage.h>

namespace otawa {
	class GenericState;
}


class FunctionalUnitConfiguration {
	bool _is_pipelined;
	int _latency;
	int _width;
	elm::genstruct::SLList<instruction_type_t> instruction_types;
	
	public:
		FunctionalUnitConfiguration(bool is_pipelined, int latency, int width);
		bool isPipelined();
		int latency();
		int width();
		void addInstructionType(instruction_type_t type);
		elm::genstruct::SLList<instruction_type_t> * instructionTypes();
};

class FunctionalUnit {
	bool _is_pipelined;
	int _latency;
	int _width;
	int pending_instructions;
	int new_instructions;
	
	
	public:
		FunctionalUnit(bool is_pipelined, int latency, int width);
		bool isPipelined();
		int latency();
		int width();
		inline int newInstructions();
		inline int pendingInstructions();
		inline void addInstruction();
		inline void subInstruction();
		inline void resetNewInstructions();
};

inline int FunctionalUnit::newInstructions() {
	return new_instructions;
}

inline int FunctionalUnit::pendingInstructions() {
	return pending_instructions;
}

inline void FunctionalUnit::addInstruction() {
	new_instructions++;
	pending_instructions++;
}


inline void FunctionalUnit::subInstruction() {
	pending_instructions--;
	assert(pending_instructions >= 0);
}

inline void FunctionalUnit::resetNewInstructions() {  
	new_instructions = 0;
}



class ExecuteInOrderStageIQ : public PipelineStage {
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
		elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
		elm::genstruct::AllocatedTable<FunctionalUnit *> * functional_units;
		elm::genstruct::AllocatedTable<FunctionalUnit *> * fu_bindings;
		int number_of_functional_units;
		elm::genstruct::SLList<SimulatedInstruction *> executing_instructions;
		
		
	public:
		ExecuteInOrderStageIQ(sc_module_name name, int width, otawa::GenericState * gen_state,
								elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
								elm::genstruct::SLList<FunctionalUnitConfiguration *> * _functional_units);
		
		SC_HAS_PROCESS(ExecuteInOrderStageIQ);
		void action();
};

class ExecuteOOOStage : public PipelineStage {
	public:
		// signals
		sc_in<bool> in_clock;
		
	private:
		// variables
		int stage_width;
		elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
		InstructionQueue * rob;
		elm::genstruct::AllocatedTable<FunctionalUnit *> * functional_units;
		elm::genstruct::AllocatedTable<FunctionalUnit *> * fu_bindings;
		int number_of_functional_units;
		
	public:
		ExecuteOOOStage(sc_module_name name, int width, InstructionQueue * _rob,
								elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
								elm::genstruct::SLList<FunctionalUnitConfiguration *> * _functional_units);
		
		SC_HAS_PROCESS(ExecuteOOOStage);
		void action();
};

class CommitStage : public PipelineStage {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_in<SimulatedInstruction *> * in_instruction;
		sc_in<int> in_number_of_ins;
		sc_out<int> out_number_of_accepted_ins;
		
	private:
		// variables
		int width;
		otawa::GenericState * sim_state;
		
	
	public:
		CommitStage(sc_module_name name, int _width, otawa::GenericState * gen_state);
		
		SC_HAS_PROCESS(CommitStage);
		void action();
};



#endif //_EXECUTE_H_
