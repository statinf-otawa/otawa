/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_GENSIM_INSTRUCTION_QUEUE_H
#define OTAWA_GENSIM_INSTRUCTION_QUEUE_H

#include "SimulatedInstruction.h"
#include <systemc.h>
#include <elm/string/CString.h>
#include <otawa/util/Trace.h>

namespace otawa {
namespace gensim {

class InstructionQueueConfiguration {
	CString queue_name;
	int cap;
	int number_of_write_ports;
	int number_of_read_ports;
	simulated_instruction_state_t leaving_condition;
public:
	InstructionQueueConfiguration(CString queue_name, int capacity,
			simulated_instruction_state_t condition);
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
	Trace *_trace;

	// variables
	bool is_full;
	int head;
	int tail;
	int cap;
	int in_ports, out_ports;
	SimulatedInstruction** buffer;

	// private methods
	SimulatedInstruction* get();
	void put(SimulatedInstruction *inst);

	SC_HAS_PROCESS(InstructionQueue);
	void action();

public:
	InstructionQueue(sc_module_name name,
			InstructionQueueConfiguration * configuration, Trace *trace);
	~InstructionQueue();
	void flush();
	SimulatedInstruction* read(int index);
	InstructionQueueConfiguration * configuration();
	bool isEmpty();
	int size();

};

}
} // otawa::gensim

#endif // OTAWA_GENSIM_INSTRUCTION_QUEUE_H
