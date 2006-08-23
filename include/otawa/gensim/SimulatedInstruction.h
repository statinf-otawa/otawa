#ifndef _SIMULATEDINSTRUCTION_H_
#define _SIMULATEDINSTRUCTION_H_

#include <otawa/otawa.h>
#include <emul.h>

typedef enum {WAITING, READY, EXECUTED, TERMINATED} simulated_instruction_state_t; // ordered set

class SimulatedInstruction {
	private:
		otawa::Inst * instruction;
		code_t binary_code;
		instruction_t * emulated_inst;
		simulated_instruction_state_t instruction_state;
	public:
		inline SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst);
		inline otawa::Inst * inst();
		inline instruction_t * emulatedInst();
		inline void setState(simulated_instruction_state_t new_state);
		inline simulated_instruction_state_t state();
};

inline SimulatedInstruction::SimulatedInstruction(otawa::Inst* inst, code_t code, instruction_t* emul_inst) :
	instruction(inst), binary_code(code), emulated_inst(emul_inst) {
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

#endif //_SIMULATEDINSTRUCTION_H_
