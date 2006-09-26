/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/gensim/InstructionQueue.h -- InstructionQueue class interface.
 */
#ifndef OTAWA_GENSIM_INSTRUCTION_QUEUE_H
#define OTAWA_GENSIM_INSTRUCTION_QUEUE_H

#include <otawa/gensim/SimulatedInstruction.h>
#include <systemc.h>
#include <elm/string/CString.h>

namespace otawa { namespace gensim {

class InstructionQueueConfiguration {
	int cap;
	int number_of_write_ports;
	int number_of_read_ports;
	CString queue_name;
	simulated_instruction_state_t leaving_condition;
	public :
		InstructionQueueConfiguration(CString queue_name, int capacity, simulated_instruction_state_t condition);
		int capacity();
		int numberOfWritePorts();
		int numberOfReadPorts();
		void setNumberOfWritePorts(int n);
		void setNumberOfReadPorts(int n);
		simulated_instruction_state_t leavingCondition();
		CString name();
		void dump(elm::io::Output& out_stream);
};

class InstructionQueue : public sc_module {
	friend class InstructionBuffer;
	public:
		// signals
		sc_in<bool> in_clock;
		sc_in<SimulatedInstruction *> * in_instruction;
		sc_in<int> in_number_of_ins;
		sc_out<SimulatedInstruction *> * out_instruction;
		sc_out<int> out_number_of_outs;
		sc_out<int> out_number_of_accepted_ins;
		sc_in<int> in_number_of_accepted_outs;
	
	private:
		// parameters
		InstructionQueueConfiguration * conf;
		
		// variables
		int head;
		int tail;
		bool is_full;
		int cap;
		int in_ports, out_ports;
		SimulatedInstruction** buffer;
		
		// private methods
		SimulatedInstruction* get();
		void put(SimulatedInstruction *inst);
		
		SC_HAS_PROCESS(InstructionQueue);
		void action();
	
	public:
		InstructionQueue(sc_module_name name, InstructionQueueConfiguration * configuration);
		~InstructionQueue(); 
		void flush();
		SimulatedInstruction* read(int index);
		InstructionQueueConfiguration * configuration();
		bool isEmpty();
		int size();
		
};

} } // otawa::gensim

#endif // OTAWA_GENSIM_INSTRUCTION_QUEUE_H
