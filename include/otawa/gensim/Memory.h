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

#ifndef OTAWA_GENSIM_MEMORY_H
#define OTAWA_GENSIM_MEMORY_H

/* #include "PipelineStage.h" */
/* #include "InstructionQueue.h" */
/* #include "SimulatedInstruction.h" */
#include <otawa/otawa.h>
#include "SimulationStats.h"
#include <otawa/util/Trace.h>
#include "GenericState.h"
#include <otawa/sim/CacheDriver.h>
#include <otawa/sim/AbstractCacheDriver.h>

namespace otawa {
	
namespace hard { class Memory; }
	
namespace gensim {

class GenericState;

class MemoryStats : public SimulationStats {
	int _number_instruction_cache_hits;
	int _number_instruction_cache_misses;
	int _number_instruction_accesses;
	int _number_data_cache_hits;
	int _number_data_cache_misses;
	int _number_data_reads;
	int _number_data_accesses;
	int _number_encoding_instruction_decompression;
	int _number_instruction_register_hits;
	int _number_compressed_instruction_accesses;
public:
	inline MemoryStats() :
	_number_instruction_cache_hits(0),
	_number_instruction_cache_misses(0),
	_number_instruction_accesses(0),
	_number_data_cache_hits(0),
	_number_data_cache_misses(0),
	_number_data_reads(0),
	_number_data_accesses(0),
	_number_encoding_instruction_decompression(0),
	_number_instruction_register_hits(0),
	_number_compressed_instruction_accesses(0) {
	}
	
	inline void incrementNumberCompressedInstructionAccesses() {
		_number_compressed_instruction_accesses++;
	}
	inline void incrementNumberEncodingInstructionDecompression() {
		_number_encoding_instruction_decompression++;
	}
	inline void incrementNumberInstructionRegisterHits() {
		_number_instruction_register_hits++;
	}
	inline void incrementNumberInstructionCacheHits() {
		_number_instruction_cache_hits++;
	}
	inline void incrementNumberInstructionCacheMisses() {
		_number_instruction_cache_misses++;
	}
	inline void incrementNumberInstructionAccesses() {
		_number_instruction_accesses++;
	}
	inline void incrementNumberDataCacheHits() {
		_number_data_cache_hits++;
	}
	inline void incrementNumberDataCacheMisses() {
		_number_data_cache_misses++;
	}
	inline void incrementNumberDataAccesses() {
		_number_data_accesses++;
	}
	inline void incrementNumberDataReads() {
		_number_data_reads++;
	}
	
	inline int getNbrInstructionAccesses() const {
		return _number_instruction_accesses;
	}

	inline int getNbrInstructionCacheHits() const {
			return _number_instruction_cache_hits;
	}

	inline int getNbrInstructionCacheMisses() const {
			return _number_instruction_cache_misses;
	}

	inline int getNbrDataAccesses() const {
			return _number_data_accesses;
	}

	inline int getNbrDataReads() const {
			return _number_data_reads;
	}

	inline int getNbrDataWrites() const {
			int Nbr_writes = 0;
			
			if (_number_data_accesses)
				Nbr_writes = _number_data_accesses - _number_data_reads;
			return Nbr_writes;
	}

	inline int getNbrDataCacheHits() const {
			return _number_data_cache_hits;
	}

	inline int getNbrDataCacheMisses() const {
			return _number_data_cache_misses;
	}

	virtual inline void reset(void) {
		_number_instruction_cache_hits = 0;
		_number_instruction_cache_misses = 0;
		_number_instruction_accesses = 0;
		_number_data_cache_hits = 0;
		_number_data_cache_misses = 0;
		_number_data_reads = 0;
		_number_data_accesses = 0;
		_number_encoding_instruction_decompression = 0;
		_number_instruction_register_hits = 0;
		_number_compressed_instruction_accesses = 0;
	}

	void dump(elm::io::Output& out_stream);
	//static int getNumberOfInstructionAccesses(); // XXXXXXXXXXXX
	
};


class MemorySystem : public sc_module {
public:
	// signals
	sc_in<bool> in_clock;
	sc_in<address_t> in_inst_address;
	sc_in<bool> in_inst_request;
	sc_out<bool> out_inst_wait;
	sc_in<address_t> in_data_address;
	sc_in<otawa::sim::CacheDriver::action_t> in_data_access_type;
	sc_in<int> in_data_size;
	sc_in<bool> in_data_request;
	sc_out<bool> out_data_wait;
	MemoryStats _stats1;

private:
	GenericState *sim_state;
	MemoryStats _stats;
	Trace *_trace;
	typedef enum {READY=0, BUSY=1} memory_state_t;
	memory_state_t _inst_cache_state;
	int _inst_fill_latency;
	memory_state_t _data_cache_state;
	const hard::Memory *mem;

	int getLatency(Address address) const;
	bool isCached(Address address) const;
	int _data_fill_latency;
	void processInstCache();
	void processDataCache();
	void processDataSPM();

public:

	MemorySystem(
		sc_module_name name,
		GenericState * gen_state,
		Trace *trace,
		const hard::Memory *mem);

	SC_HAS_PROCESS(MemorySystem);
	void action();

};

}
} // otawa::gensim

#endif // OTAWA_GENSIM_FETCH_H
