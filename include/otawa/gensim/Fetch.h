/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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

#ifndef OTAWA_GENSIM_FETCH_H
#define OTAWA_GENSIM_FETCH_H

#include "PipelineStage.h"
#include "InstructionQueue.h"
#include "SimulatedInstruction.h"
#include <otawa/otawa.h>
#include "SimulationStats.h"
#include <otawa/util/Trace.h>
#include "GenericState.h"
#include <otawa/sim/CacheDriver.h>
#include <otawa/sim/AbstractCacheDriver.h>

#define INST_CODE_SIZE 4

namespace otawa {
  namespace gensim {

    class GenericState;

#define NB_INT_REGS 32
#define NB_FP_REGS 32
    // FIXME: should be learned from gliss

    class FetchStats : public SimulationStats {
      int _number_fetched_instructions;
      int _number_cache_accesses;
      int _number_total_fetched_instructions;
    public:
      inline FetchStats() :
	_number_fetched_instructions(0), _number_cache_accesses(0),_number_total_fetched_instructions(0) {
      }
      inline void incrementNumberFetchedInstructions() {
	_number_fetched_instructions++;
      }
      inline void incrementTotalNumberFetchedInstructions() {
	_number_total_fetched_instructions++;
      }
      inline void incrementNumberCacheAccesses() {
	_number_cache_accesses++;
      }
      virtual inline void reset(void) {
    	  _number_fetched_instructions = 0;
    	  _number_cache_accesses = 0;
    	  _number_total_fetched_instructions = 0;
      }
      void dump(elm::io::Output& out_stream);
    };

    class FetchStage : public PipelineStage {
    public:
      // signals
      sc_in<bool> in_clock;
      sc_out<SimulatedInstruction *> * out_fetched_instruction;
      sc_out<int> out_number_of_fetched_instructions;
      sc_in<int> in_number_of_accepted_instructions;
      // interface to the instruction cache
      sc_out<address_t> out_address;
      sc_out<bool> out_request;
      sc_in<bool> in_wait;

    private:
      GenericState *sim_state;
      unsigned int out_ports;
      elm::genstruct::SLList<SimulatedInstruction *> fetched_instructions;
      elm::genstruct::SLList<SimulatedInstruction *> * active_instructions;

      // variables
      int _inst_index;
      otawa::Inst* _next_inst;
      SimulatedInstruction* _inst;
      address_t _last_line_address;
      elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
      FetchStats stats;
      Trace *_trace;
      typedef enum {READY=0, WAITING=1} fetch_state_t;
      fetch_state_t _fetch_state;
      unsigned int _nb_fetched;
      bool *_ended;

    public:

      FetchStage(
		 sc_module_name name,
		 int number_of_out_ports,
		 GenericState * gen_state,
		 elm::genstruct::AllocatedTable<rename_table_t> * rename_tables,
		 elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions,
		 bool *ended);
      FetchStage(
		 sc_module_name name,
		 int number_of_out_ports,
		 GenericState * gen_state,
		 elm::genstruct::AllocatedTable<rename_table_t> * rename_tables,
		 elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions,
		 bool *ended, Trace *trace);

      inline void GetInstruction() {
	_inst = new SimulatedInstruction(_next_inst, _inst_index++, active_instructions, sim_state);
	_inst->renameOperands(rename_tables);
	fetched_instructions.addLast(_inst);
	_nb_fetched++;
	stats.incrementNumberFetchedInstructions();

      }

      inline void InstCacheRequest(address_t addr) {
	out_address.write(addr);
	out_request.write(true);
	stats.incrementNumberCacheAccesses();
      }

      inline bool InstCacheWait() {
	return in_wait.read();
      }
      void fetch();

      SC_HAS_PROCESS(FetchStage);

      void action();


    };

  }
} // otawa::gensim

#endif // OTAWA_GENSIM_FETCH_H
