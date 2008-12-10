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

#include <otawa/gensim/Memory.h>
#include <otawa/gensim/GenericState.h>
#include <otawa/hard/Memory.h>
#include <otawa/gensim/GenericSimulator.h>

namespace otawa {
  namespace gensim {

    //<<<<<<< Memory.cpp
    void MemoryStats::dump(elm::io::Output& out_stream) {
      out_stream << "\n---- Stats from the memory system ----\n";
      out_stream << "number of instruction cache accesses: "
		 << _number_instruction_accesses << "\n";
      out_stream << "number of instruction cache hits: "
		 << _number_instruction_cache_hits << "\n";
      out_stream << "number of instruction cache misses: "
		 << _number_instruction_cache_misses << "\n";
      out_stream << "number of instruction register hits: "
		 << _number_instruction_register_hits << "\n";
      out_stream << "number of encoding instruction decompression: "
		 << _number_encoding_instruction_decompression << "\n";
      out_stream << "number of compressed instruction accesses: "
		 << _number_compressed_instruction_accesses << "\n";
      out_stream << "number of data accesses: " << _number_data_accesses << "\n";
      out_stream << "number of data reads: " << _number_data_reads << "\n";
      int writes = 0;
      if (_number_data_accesses)
	writes = _number_data_accesses - _number_data_reads;
      out_stream << "number of data writes: " << writes << "\n";
      out_stream << "number of data cache hits: " << _number_data_cache_hits
		 << "\n";
      out_stream << "number of data cache misses: " << _number_data_cache_misses
		 << "\n";

    }


    /**
     * Constructor.
     */
    MemorySystem::MemorySystem(
			       sc_module_name name,
			       GenericState * gen_state,
			       Trace *trace,
			       const hard::Memory *memory
			       ):
      _inst_cache_state(READY),
      _inst_fill_latency(0),
      _data_cache_state(READY),
      mem(memory)
    {
      gen_state->addStats(&_stats);
      _trace = trace;
      sim_state = gen_state;
      SC_METHOD(action);
      sensitive_neg << in_clock;
    }


    /**
     * Process access to the instruction cache.
     */
    void MemorySystem::processInstCache() {
      switch (_inst_cache_state) {

      case READY:
	if (in_inst_request.read() == true) {
	  address_t address = in_inst_address.read();			
	  otawa::sim::CacheDriver::result_t res = sim_state->icache_driver->access(address, 4, otawa::sim::CacheDriver::READ);

	  switch(res) {
	  case  (otawa::sim::CacheDriver::HIT): 
	    //elm::cerr << "hit for " << address << io::endl;
	    out_inst_wait.write(false);
	    _stats.incrementNumberInstructionAccesses();
	    _stats.incrementNumberInstructionCacheHits();
	    break;
				
	  case (otawa::sim::CacheDriver::MISS):
	    //elm::cerr << "miss for " << address << io::endl;
	    _inst_fill_latency =  getLatency(address);
	    _inst_cache_state = BUSY;
	    out_inst_wait.write(true);
	    _stats.incrementNumberInstructionAccesses();
	    _stats.incrementNumberInstructionCacheMisses();
	    break;
			
				
	  default: 
	    ASSERTP(false, "PB dans gestion fonctionnelle du  cache");
	  }
	}
	break;
		
      case BUSY:
	_inst_fill_latency--;
	if (_inst_fill_latency == 0) {
	  out_inst_wait.write(false);
	  _inst_cache_state = READY;
	}
	break;
      }

    }

    void MemorySystem::processDataSPM() {
      if (in_data_request.read() == true) {
	address_t address = in_data_address.read();
	if (sim_state->mem_driver->locate(address)
	    == otawa::sim::MemoryDriver::DATA_SPM) {
	}
      }
    }


    /**
     * Process access ot the data cache.
     */
    void MemorySystem::processDataCache() {
      switch (_data_cache_state) {

      case READY:
	if (in_data_request.read() == true) {
	  _stats.incrementNumberDataAccesses();
	  address_t address = in_data_address.read();
	  //if(sim_state->mem_driver->locate(address) == sim::MemoryDriver::DATA_CACHE) {
	  if(isCached(address)) {
	    int size = in_data_size.read();
	    otawa::sim::CacheDriver::action_t type = in_data_access_type.read();
	    if(type == otawa::sim::CacheDriver::READ)
	      _stats.incrementNumberDataReads();
	    if(sim_state->dcache_driver->access(address, size, type) == sim::CacheDriver::HIT) {
	      out_data_wait.write(false);
	      _stats.incrementNumberDataCacheHits();
	    }
	    else { // cache miss
	      _data_fill_latency = getLatency(address);
	      _data_cache_state = BUSY;
	      out_data_wait.write(true);
	      _stats.incrementNumberDataCacheMisses();
	    }
	  }
	  else {
	    _data_fill_latency = getLatency(address);
	    _data_cache_state = BUSY;
	    out_data_wait.write(true);
	  } 
	}
	break;
		
      case BUSY:
	_data_fill_latency--;
	if (_data_fill_latency == 0) {
	  out_data_wait.write(false);
	  _data_cache_state = READY;
	}
	break;
      }

    }


    /**
     */
    void MemorySystem::action() {
      *_trace << Level(5) << "[MemorySystem] (cycle " << sim_state->cycle()
	      << ")\n";
      processInstCache();
      processDataCache();
      //processDataSPM();
    }


    /**
     * Get the latency of whole access to the memory.
     * @param address	Accessed address.
     * @return			Matching latency.
     */
    int MemorySystem::getLatency(Address address) const {
      const hard::Bank *bank = mem->get(address);
      ASSERT(bank);
      return bank->latency();
    }

    bool MemorySystem::isCached(Address address) const {
      const hard::Bank *bank = mem->get(address);
      if(!bank)
    	  throw gensim::Exception(_ << "access out of defined banks for address " << address);
      return bank->isCached();
    }


} } // otawa::gensim
