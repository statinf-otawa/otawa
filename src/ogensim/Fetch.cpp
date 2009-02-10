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

#include <otawa/gensim/Fetch.h>
#include <otawa/gensim/GenericState.h>



namespace otawa {
namespace gensim {

void FetchStats::dump(elm::io::Output& out_stream) {
  out_stream << "\n---- Stats from the fetch stage ----\n";
  out_stream << "number of fetched instructions: "
	     << _number_fetched_instructions << "\n";
  out_stream << "number of cache accesses: " << _number_cache_accesses
	     << "\n";
}

FetchStage::FetchStage(sc_module_name name, int number_of_out_ports,
		       GenericState * gen_state,
		       elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
		       elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions,
		       bool *ended) :
  PipelineStage(name), _inst_index(0), _next_inst(NULL), _last_line_address(0), _ended(ended) {
  gen_state->addStats(&stats);
  _fetch_state = READY;
  out_fetched_instruction
    = new sc_out<SimulatedInstruction *>[number_of_out_ports];
  out_ports = number_of_out_ports;
  sim_state = gen_state;
  rename_tables = _rename_tables;
  active_instructions = _active_instructions;
   SC_METHOD(fetch);
  sensitive_pos << in_clock;
}

FetchStage::FetchStage(sc_module_name name, int number_of_out_ports,
		       GenericState * gen_state,
		       elm::genstruct::AllocatedTable<rename_table_t> * _rename_tables,
		       elm::genstruct::SLList<SimulatedInstruction *> * _active_instructions,
		       bool *ended, Trace *trace) 
  : PipelineStage(name), _inst_index(0), _next_inst(NULL), _last_line_address(0), _ended(ended) {
  _fetch_state = READY;
  gen_state->addStats(&stats);
  _trace = trace;
  out_fetched_instruction
    = new sc_out<SimulatedInstruction *>[number_of_out_ports];
  out_ports = number_of_out_ports;
  sim_state = gen_state;
  rename_tables = _rename_tables;
  active_instructions = _active_instructions;
  SC_METHOD(fetch);
  sensitive_pos << in_clock;
}
  
void FetchStage::fetch() {
  bool stop = false;
  *_trace << Level(5) << "[FetchStage] (cycle " << sim_state->cycle() << ")\n";
  
  *_trace << L8 << "\t\tnumber of insts accepted by the destination queue = "
	  << in_number_of_accepted_instructions.read() << "\n";
  for (int i=0; i<in_number_of_accepted_instructions.read() ; i++)
    fetched_instructions.removeFirst();
  _nb_fetched = fetched_instructions.count();
  if (_nb_fetched == out_ports)
    stop = true;

  *_trace << L8
	  << "\t\tnumber of instructions already fetched (and not yet in the destination queue = "
	  << _nb_fetched << "\n";
  
  //reinit cache request to false
  out_request.write(false);
  
  address_t addr;
  if (_next_inst == NULL) {
    _next_inst = sim_state->driver->nextInstruction(*sim_state, _next_inst);
    if (_next_inst != NULL) {
      addr = _next_inst->address();
      
      *_trace << L6 << "\t\trequest cache line for next inst (@" << addr
	      << ")\n";
      InstCacheRequest(addr);
      _last_line_address = (addr / (out_ports*INST_CODE_SIZE))
	* (out_ports*INST_CODE_SIZE);
      _fetch_state = WAITING;
      stop = true;
    } else {
      stop = true;
      *_ended = true;
    }
  }
  
  while ( !stop) {
    switch (_fetch_state) {
    case READY:
      *_trace << L5 << "\tfetching at @" << addr << "\n";
      GetInstruction();
      if (_inst->isControl()) {
	stop = true;
	*_trace << L6
		<< "\t\tstop fetching after a control instruction\n";
      }
      if (_nb_fetched == out_ports)
	stop = true;
      
      _next_inst = sim_state->driver->nextInstruction(*sim_state,
						      _next_inst);
      if (_next_inst == NULL)
	stop = true;
      else {
	addr = _next_inst->address();
	if ((addr < _last_line_address) 
	    || 
	    (addr - _last_line_address  >= out_ports*INST_CODE_SIZE)) { // next inst is in a new line
	  *_trace << L6 << "\t\trequest cache line for next inst (@"
		  << addr << ")\n";
	  InstCacheRequest(addr);
	  _last_line_address = (addr / (out_ports*INST_CODE_SIZE))
	    * (out_ports*INST_CODE_SIZE);
	  _fetch_state = WAITING;
	  stop = true;
	}
      }
      break;
    case WAITING:
      if (InstCacheWait()) {
	stop = true;
	*_trace << L6 << "\t\tstill waiting for the cache\n";
      } else {
	*_trace << L6 << "\t\trequested cache line is here\n";
	*_trace << L5 << "\tfetching at @" << _next_inst->address()
		<< "\n";
	GetInstruction();
	if (_inst->isControl()) {
	  stop = true;
	  *_trace << L6
		  << "\t\tstop fetching after a control instruction\n";
	}
	if (_nb_fetched == out_ports)
	  stop = true;
	_fetch_state = READY;
	_next_inst = sim_state->driver->nextInstruction(*sim_state,
							_next_inst);
	if (_next_inst == NULL) {
	  stop = true;
	  *_ended = true;
	} else {
	  addr = _next_inst->address();
	  if ((addr < _last_line_address) || (addr
					      - _last_line_address >= out_ports*INST_CODE_SIZE)) { // next inst is in a new line
	    *_trace << L6
		    << "\t\trequest cache line for next inst (@"
		    << addr << ")\n";
	    InstCacheRequest(addr);
	    _last_line_address
	      = (addr / (out_ports*INST_CODE_SIZE))
	      * (out_ports*INST_CODE_SIZE);
	    _fetch_state = WAITING;
	    stop = true;
	  }
	}
	
      }
      break;
    }
  }
  
  unsigned int outs = 0;
  for (elm::genstruct::SLList<SimulatedInstruction *>::Iterator
	 inst(fetched_instructions); inst && (outs < out_ports); inst++) {
    out_fetched_instruction[outs++] = inst;
  }
  out_number_of_fetched_instructions.write(outs);
  *_trace << L8
	  << "\t\tnumber of instructions submitted to the destination queue) = "
	  << outs << "\n";
}
  
}}// otawa::gensim

