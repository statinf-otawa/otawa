/*
 * $Id$
 * Copyright (c) 2006, IRIT-UPS
 *
 * otawa/gensim/Fetch.h -- FetchStage class interface
 */
#ifndef OTAWA_GENSIM_FETCH_H
#define OTAWA_GENSIM_FETCH_H

#include <otawa/gensim/PipelineStage.h>
#include <otawa/gensim/InstructionQueue.h>	
#include <otawa/gensim/SimulatedInstruction.h>
//#include <emul.h>
//#include <iss_include.h>
#include <otawa/otawa.h>

namespace otawa { namespace gensim {

// External class
class GenericState;

#define NB_INT_REGS 32
#define NB_FP_REGS 32
	// FIXME: should be learned from gliss

class FetchStage : public PipelineStage {
	public:
		// signals
		sc_in<bool> in_clock;
		sc_out<SimulatedInstruction *> * out_fetched_instruction;
		sc_out<int> out_number_of_fetched_instructions;
		sc_in<int>in_number_of_accepted_instructions;
		// interface to the instructiono cache
		sc_out<address_t> out_address;
		sc_out<int> out_requested_bytes;
		sc_in<bool> in_hit;
		sc_in<int> in_delivered_bytes;
		
	
	private:	
		// parameters
		GenericState *sim_state;
		//state_t * emulated_state;
		int out_ports;
		elm::genstruct::SLList<SimulatedInstruction *> fetched_instructions;
		elm::genstruct::SLList<SimulatedInstruction *> * active_instructions;
		
		// variables
		otawa::Inst* next_inst;
		elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
		
	public:

		FetchStage(sc_module_name name, int number_of_out_ports, GenericState * gen_state,
					elm::genstruct::AllocatedTable<rename_table_t> * rename_tables,
					elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions);

		SC_HAS_PROCESS(FetchStage);	
		void fetch();

};

} } // otawa::gensim

#endif // OTAWA_GENSIM_FETCH_H
