/*
 *	$Id$
 *	exegraph module implementation
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

#include <otawa/gensim/Execute.h>
#include <otawa/gensim/GenericState.h>

namespace otawa {
namespace gensim {

  SimulatedInstruction * FunctionalUnit::getFinishedInstruction() {
     for (int i=0 ; i<_width; i++) {
       int stage = 0;
       if (_is_pipelined) 
	 stage = _latency-1;
       SimulatedInstruction *inst = _instructions[i][stage];
       if (inst) {
	 if (inst->fuState() == TERMINATED) {
	   _instructions[i][stage] = NULL;
	   return(inst);
	 }
       }
       // return(NULL);
     }
     return(NULL);
   }

   bool FunctionalUnit::issueInstruction(SimulatedInstruction *inst){
     bool found = false;
     for (int i=0 ; (i<_width) && !found ; i++) {
       if (_instructions[i][0] == NULL) {
	 found = true;
	 _instructions[i][0] = inst;
	 inst->setFuState(PENDING);
	 if (_is_pipelined)
	   inst->setTimeToFinish(1);
	 else
	   inst->setTimeToFinish(_latency);
       }
     }
     return(found);
   }

   void FunctionalUnit::issueMemoryAccess(SimulatedInstruction *inst) {
     inst->setFuState(ACCESSING_MEMORY);
     _pending_memory_accesses++;
     address_t addr;
     otawa::sim::CacheDriver::action_t access_type;
     if (inst->inst()->isLoad()) {
       access_type =  otawa::sim::CacheDriver::READ;
       addr = inst->getReadMemAddr();
     }
     else { //store
       access_type =  otawa::sim::CacheDriver::WRITE;
       addr = inst->getWriteMemAddr();
     }
     _execute_stage->DataCacheRequest(addr, access_type);
   }

   void FunctionalUnit::checkMemoryAccess(SimulatedInstruction *inst) {
     bool ok = !_execute_stage->DataCacheWait(); 
     if (ok) {
       _pending_memory_accesses --;
       assert(_pending_memory_accesses >= 0);
       inst->setFuState(TERMINATED);
     }
   }

   void FunctionalUnit::execute() {
     for (int i=0 ; i<_width ; i++) {
       if (_is_pipelined) {
	 for (int j=_latency-2 ; j>=0 ; j--) { // all the stages but the last one
	   SimulatedInstruction *inst = _instructions[i][j];
	   if (inst != NULL) {
	     if (_instructions[i][j+1] == NULL){
	       _instructions[i][j+1] = _instructions[i][j];
	       _instructions[i][j] = NULL;
	     }
	   }
	 }
	 SimulatedInstruction *inst = _instructions[i][_latency-1]; // instruction in the last stage
	 if (inst != NULL) {
	   switch(inst->fuState()) {
	   case PENDING:
	     if (inst->inst()->isLoad() || inst->inst()->isStore()) {
	       if (_pending_memory_accesses == 0 /* FIXME */)
		 issueMemoryAccess(inst);
	     }
	     else {
	       inst->setFuState(TERMINATED);
	     }
	     break;
	   case ACCESSING_MEMORY:
	     checkMemoryAccess(inst);
	     break;
	   case TERMINATED:
	   default:
	     break;
	   }
	 }
       }
       else { // not pipelined
	 SimulatedInstruction *inst = _instructions[i][0];
	 if (inst != NULL) {
	   switch(inst->fuState()) {
	   case PENDING:
	     if (inst->inst()->isLoad() || inst->inst()->isLoad()) {
	       if (_pending_memory_accesses == 0 /* FIXME */)
		 issueMemoryAccess(inst);
	     }
	     else {
	       inst->decrementTimeToFinish();
	       if (inst->timeToFinish() == 0)
		 inst->setFuState(TERMINATED);
	     }
	     break;
	   case ACCESSING_MEMORY:
	     checkMemoryAccess(inst);
	     break;
	   case TERMINATED:
	   default:
	     break;
	   }
	 }
       }
     }
   }

  void ExecuteStats::dump(elm::io::Output& out_stream) {
    out_stream << "\n---- Stats from the execute stage ----\n";
    out_stream << "number of executed instructions: "
	       << _number_executed_instructions << "\n";
    out_stream << "number of accesses to the data cache: "
	       << _number_cache_accesses << "\n";
  }



  ExecuteOOOStage::ExecuteOOOStage(
				   sc_module_name name,
				   int width,
				   bool in_order,
				   InstructionQueue * _rob,
				   GenericState *gen_state,
				   elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
				   elm::genstruct::SLList<FunctionalUnitConfiguration *> * _functional_units,
				   elm::genstruct::Vector<elm::Pair<Inst::kind_t, FunctionalUnitConfiguration *> > *fu_conf_bindings,
				   Trace *trace) :
    PipelineStage(name),stage_width(width), _execute_in_order(in_order), _trace(trace), rob(_rob),
    rename_tables(_rename_tables), sim_state(gen_state)
     {
    gen_state->addStats(&stats);
    int fu_number = _functional_units->count();
    functional_units = new elm::genstruct::AllocatedTable<FunctionalUnit *>(fu_number);
    //int inst_type_number= INST_TYPE_NUMBER /*FIXME*/;
    number_of_functional_units = 0;
    int fu_index = 0;
    elm::genstruct::Vector<elm::Pair<FunctionalUnit *, FunctionalUnitConfiguration *> >
      fu_conf_relation;
    for (elm::genstruct::SLList<FunctionalUnitConfiguration *>::Iterator
	   fu_conf(*_functional_units); fu_conf; fu_conf++) {
      FunctionalUnit * fu = new FunctionalUnit(fu_conf->name(), fu_conf->isPipelined(), fu_conf->latency(), fu_conf->width(), fu_index, this);
      fu_conf_relation.add(elm::Pair<FunctionalUnit *, FunctionalUnitConfiguration *> (fu, fu_conf));
      (*functional_units)[number_of_functional_units++] = fu;
      fu_index++;
    }
    for (elm::genstruct::Vector<elm::Pair<Inst::kind_t, FunctionalUnitConfiguration *> >::Iterator binding(*fu_conf_bindings); binding; binding++) {
      bool found = false;
      FunctionalUnit *fu = NULL;
      for (elm::genstruct::Vector<elm::Pair<FunctionalUnit *, FunctionalUnitConfiguration *> >::Iterator rel(fu_conf_relation); rel && !found; rel++) {
	if (rel.item().snd == binding.item().snd) {
	  found = true;
	  fu = rel.item().fst;
	}
      }
      fu_bindings.add(Pair<Inst::kind_t, FunctionalUnit *>(binding.item().fst, fu));
    }

    SC_METHOD(action);
    sensitive_pos << in_clock;
  }


  void ExecuteOOOStage::action() {
    int executed = 0;
    bool load_pending = false;
    bool store_pending = false;
    bool stop = false;
    int i;
    *_trace << L5 << "[ExecuteStage] (cycle " << sim_state->cycle() << ")\n";
    out_request.write(false);

    for (int i=0; i<number_of_functional_units; i++) {
      SimulatedInstruction * finished_inst = (*functional_units)[i]->getFinishedInstruction();
      while (finished_inst != NULL) {
	finished_inst->notifyResult(rename_tables);
	if (finished_inst->inst()->isLoad())
	  load_pending = false;
	if (finished_inst->inst()->isStore())
	  store_pending = false;
	*_trace << L6 << "\tterminating  " << finished_inst->dump() << "\n";
	finished_inst = (*functional_units)[i]->getFinishedInstruction();
      }
    }

    for (i=0; (i<rob->size()); i++) {
      SimulatedInstruction * inst = rob->read(i);
      if (_execute_in_order && ((inst->state() == WAITING)||stop))
	break;
      if (inst->inst()->isLoad()) {
	if (inst->state() == WAITING) {
	  load_pending = true;
	  continue;
	}
	if ((inst->state() == READY) && store_pending) {
	  *_trace << L6 << "\tthis inst. cannot be issued to preserve memory ordering: " << inst->dump() << "\n";
	  load_pending = true;
	  continue;
	}
      }
      if (inst->inst()->isStore()) {
	if (inst->state() == WAITING) {
	  store_pending = true;
	  continue;
	}
	if ((inst->state() == READY) && (store_pending||load_pending)) {
	  *_trace << L6 << "\tthis inst. cannot be issued to preserve memory ordering: " << inst->dump() << "\n";
	  store_pending = true;
	  continue;
	}
      }

      if (inst->state() == READY) {
	FunctionalUnit * fu = findFU(inst->inst()->kind());
	ASSERTP(fu, "no FU for instruction at " << inst->inst()->address());
	*_trace << L6 << "\t" << inst->dump() << " (type " << inst->inst()->kind() << ") will be sent to FU " << fu->name(); // << "\n";
	if ( fu->issueInstruction(inst)) {
	  inst->setState(EXECUTING);
	  *_trace << L6 << "\tissuing " << inst->dump() << " to " << fu->name() << "\n";
	  executed++;
	  stats.incrementNumberExecutedInstructions();
	}
	else {
	  *_trace << L6 << "\tthis inst. cannot be issued (" << fu->name() << " is not available ): " << inst->dump() << "\n";
	  stop = true; // for in-order execution
	  if (inst->inst()->isStore())
	    store_pending = true;
	  if (inst->inst()->isStore())
	    load_pending = true;
	}
      }
    }

    for (int i=0; i<number_of_functional_units; i++) {
      (*functional_units)[i]->execute();
    }
  }

  void CommitStats::dump(elm::io::Output& out_stream) {
    out_stream << "\n---- Stats from the commit stage ----\n";
    out_stream << "number of committed instructions: " << _number_committed_instructions << "\n";
  }

  CommitStage::CommitStage(sc_module_name name, int _width, GenericState * gen_state, Trace *trace)
    :  PipelineStage(name), width(_width), sim_state(gen_state), _trace(trace) {
    in_instruction = new sc_in<SimulatedInstruction *>[width];
    gen_state->addStats(&stats);
    SC_METHOD(action);
    sensitive_pos << in_clock;
  }

  void CommitStage::action() {
    SimulatedInstruction* inst;
    *_trace << L5 << "[CommitStage] (cycle " << sim_state->cycle() << ")\n";
    for (int i=0; i<in_number_of_ins.read(); i++) {
      inst = in_instruction[i].read();
      assert(inst->state() == EXECUTED);
      *_trace << L6 << "\tcommitting " << inst->dump() << "\n";
      *_trace << L3 << "\tcommitting " << inst->inst()->address() << " at cycle " << sim_state->cycle() << "\n";
      stats.incrementNumberCommittedInstructions();
      sim_state->driver->terminateInstruction(*sim_state, inst->inst());
      delete inst;
    }
    out_number_of_accepted_ins.write(in_number_of_ins.read());
  }

}} // otawa::gensim
