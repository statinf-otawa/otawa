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

#ifndef OTAWA_GENSIM_EXECUTE_H
#define OTAWA_GENSIM_EXECUTE_H

#include <systemc.h>
#include "SimulatedInstruction.h"
#include <otawa/util/Trace.h>
#include "PipelineStage.h"
#include <elm/util/Pair.h>
#include <elm/genstruct/Vector.h>
#include <otawa/sim/CacheDriver.h>
#include <otawa/sim/AbstractCacheDriver.h>

namespace otawa {
namespace gensim {

class GenericState;
class FunctionalUnit;

// ----------------------------------------------------------------------
class ExecuteStats : public SimulationStats {
  int _number_executed_instructions;
  int _number_cache_accesses;
 public:
  inline ExecuteStats() :
  _number_executed_instructions(0), _number_cache_accesses(0) {
  }
  inline void incrementNumberExecutedInstructions() {
    _number_executed_instructions++;
  }
  inline void incrementNumberCacheAccesses() {
    _number_cache_accesses++;
  }
  void dump(elm::io::Output& out_stream);
};

// ----------------------------------------------------------------------
 class FunctionalUnitConfiguration {
   elm::String _name;
   bool _is_pipelined;
   int _latency;
   int _width;
   elm::genstruct::SLList<Inst::kind_t> instruction_kinds;

 public:
 FunctionalUnitConfiguration(elm::String name, bool is_pipelined,
			     int latency, int width)
   : _name(name), _is_pipelined(is_pipelined), _latency(latency), _width(width) {
       if (is_pipelined)
	 assert(latency> 1);
     }
   bool isPipelined() {
     return _is_pipelined;
   }
   elm::String name() {
     return _name;
   }
   int latency() {
     return _latency;
   }
   int width() {
     return _width;
   }
   void addInstructionKind(Inst::kind_t kind) {
     instruction_kinds.addLast(kind);
   }
   elm::genstruct::SLList<Inst::kind_t> * instructionKinds() {
     return &instruction_kinds;
   }
 };

 class ExecuteOOOStage;

// ----------------------------------------------------------------------
 class FunctionalUnit {
   String _name;
   bool _is_pipelined;
   int _latency;
   int _width;
   elm::genstruct::AllocatedTable<elm::genstruct::AllocatedTable<SimulatedInstruction *> > _instructions;
   int _pending_memory_accesses;
   ExecuteOOOStage *_execute_stage;
   

 public:
 FunctionalUnit(elm::String name, bool is_pipelined, int latency, int width,
		int index, ExecuteOOOStage *stage)
   : _name(name), _is_pipelined(is_pipelined), _latency(latency),
     _width(width), _pending_memory_accesses(0), _execute_stage(stage){
     _instructions.allocate(_width);
     for (int i=0 ; i<_width ; i++){
       if (_is_pipelined) {
	 //(*_instructions)[i] = new elm::genstruct::AllocatedTable<SimulatedInstruction *>(_latency);
	 _instructions[i].allocate(_latency);
	 for (int j=0 ; j<_latency ; j++)
	   _instructions[i][j] = NULL;
       }
       else{
	 _instructions[i].allocate(1);
	 _instructions[i][0] = NULL;
       }
     }
   }

   SimulatedInstruction * getFinishedInstruction();
   bool issueInstruction(SimulatedInstruction *inst);
   void issueMemoryAccess(SimulatedInstruction *inst);
   void checkMemoryAccess(SimulatedInstruction *inst);
   void execute();

   bool isPipelined() {
     return _is_pipelined;
   }
   int latency() {
     return _latency;
   }
   String name() {
     return _name;
   }
   int width() {
     return _width;
   }
 };


 class ExecuteOOOStage : public PipelineStage {
 public:
   sc_in<bool> in_clock;
   sc_out<address_t> out_address;
   sc_out<int> out_size;
   sc_out<otawa::sim::CacheDriver::action_t> out_access_type;
   sc_out<bool> out_request;
   sc_in<bool> in_wait;

 private:
   int stage_width;
   bool _execute_in_order;
   Trace *_trace;
   InstructionQueue * rob;
   elm::genstruct::AllocatedTable<rename_table_t> * rename_tables;
   GenericState * sim_state;
   ExecuteStats stats;
   elm::genstruct::AllocatedTable<FunctionalUnit *> * functional_units;
   elm::genstruct::Vector<elm::Pair<Inst::kind_t, FunctionalUnit *> > fu_bindings;
  int number_of_functional_units;
   inline FunctionalUnit *findFU(Inst::kind_t kind) {
     for (int i = 0; i < fu_bindings.length(); i++) {
       Inst::kind_t mask = fu_bindings[i].fst;
       if ((mask & kind) == mask) {
	 return fu_bindings[i].snd;
       }
     }
     return 0;
   }

 public:
   ExecuteOOOStage(
		   sc_module_name name,
		   int width,
		   bool in_order,
		   InstructionQueue * _rob,
		   GenericState *gen_state,
		   elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
		   elm::genstruct::SLList<FunctionalUnitConfiguration *> * _functional_units,
		   elm::genstruct::Vector<elm::Pair<Inst::kind_t, FunctionalUnitConfiguration *> > *fu_bindings,
		   Trace *trace);

   SC_HAS_PROCESS(ExecuteOOOStage);
   void action();
   inline void DataCacheRequest(address_t addr, otawa::sim::CacheDriver::action_t access_type) {
     out_address.write(addr);
     out_access_type.write(access_type);
     out_request.write(true);
     stats.incrementNumberCacheAccesses();
   }
   
   inline bool DataCacheWait() {
     return in_wait.read();
   }
   
 };

 class CommitStats : public SimulationStats {
   int _number_committed_instructions;
 public:
   inline CommitStats() :
   _number_committed_instructions(0) {
   }
   inline void incrementNumberCommittedInstructions() {
     _number_committed_instructions++;
   }
   void dump(elm::io::Output& out_stream);
 };

 class CommitStage : public PipelineStage {
 public:
   sc_in<bool> in_clock;
   sc_in<SimulatedInstruction *> * in_instruction;
   sc_in<int> in_number_of_ins;
   sc_out<int> out_number_of_accepted_ins;

 private:
   int width;
   GenericState * sim_state;
   Trace *_trace;
   CommitStats stats;

 public:
   CommitStage(sc_module_name name, int _width, GenericState * gen_state,
	       Trace *trace);
   SC_HAS_PROCESS(CommitStage);
   void action();
 };

}
} // otawa::gensim

#endif // OTAWA_GENSIM_EXECUTE_H
