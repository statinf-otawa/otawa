/*
 *	Cache class interface.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-17, IRIT UPS.
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
#ifndef OTAWA_HARD_CACHE_H
#define OTAWA_HARD_CACHE_H

#include <elm/assert.h>
#include <elm/io.h>
#include <elm/serial2/macros.h>
#include <elm/data/Vector.h>
#include <otawa/base.h>

namespace otawa { namespace hard {
	
// Cache class
class Cache {
public:
	typedef enum replace_policy_t {
		NONE = 0,
		OTHER = 1,
		LRU = 2,
		RANDOM = 3,
		FIFO = 4,
		PLRU = 5,
		MRU = 6
	} replace_policy_t;
	
	typedef enum write_policy_t {
		WRITE_THROUGH = 0,
		WRITE_BACK= 1
	} write_policity_t;
	
	typedef struct info_t {
		int access_time;
		int miss_penalty;
		int block_bits;
		int row_bits;
		int way_bits;
		replace_policy_t replace;
		write_policy_t write;
		bool allocate;
		int write_buffer_size;
		int read_port_size;
		int write_port_size;
	} info_t;

	typedef t::uint32 block_t;
	typedef t::uint32 set_t;
	typedef t::uint32 tag_t;
	typedef t::uint32 offset_t;

private:
	SERIALIZABLE(Cache,
		field("access_time", _info.access_time, 1) &
		field("miss_penalty", _info.miss_penalty, 10) &
		field("block_bits", _info.block_bits, 4) &
		field("row_bits", _info.row_bits, 12) &
		field("way_bits", _info.way_bits, 0) &
		field("allocate", _info.allocate, false) &
		field("next", _next, (const Cache *)0) &
		field("replace", _info.replace, LRU) &
		field("write_buffer_size", _info.write_buffer_size, 0) &
		field("read_port_size", _info.read_port_size, 1) &
		field("write_port_size", _info.write_port_size, 1) &
		field("write", _info.write, WRITE_THROUGH));
	info_t _info;
	const Cache *_next;

public:

	Cache(void);
	Cache(const Cache& cache, const Cache *next = 0);
	virtual ~Cache(void);
	
	// Simple accessors
	inline const Cache *nextLevel(void) const  { return _next; }
	inline ot::size cacheSize(void) const { return 1 << (blockBits() + rowBits() + wayBits()); }
	inline ot::size blockSize(void) const { return 1 << blockBits(); }

	inline int wayCount(void) const { return 1 << wayBits(); }
	inline int setCount(void) const { return 1 << rowBits(); }
	inline int blockCount(void) const { return 1 << (_info.row_bits + _info.way_bits); }

	inline replace_policy_t replacementPolicy(void) const { return _info.replace; }
	inline write_policy_t writePolicy(void) const { return _info.write; }
	inline bool doesWriteAllocate(void) const { return _info.allocate; }
	inline int missPenalty(void) const { return _info.miss_penalty; }
	inline int writeBufferSize(void) const { return _info.write_buffer_size; }
	inline int readPortSize(void) const { return _info.read_port_size; }
	inline int writePortSize(void) const { return _info.write_port_size; }
	
	// Low-level information
	inline int blockBits(void) const { return _info.block_bits; }
	inline int setBits(void) const { return _info.row_bits; }
	inline int tagBits(void) const { return 32 - blockBits() + rowBits(); }
	inline int wayBits(void) const { return _info.way_bits; }

	inline ot::mask blockMask(void) const { return blockSize() - 1; }
	inline ot::mask setMask(void) const { return (rowCount() - 1) << blockBits(); }
	inline ot::mask tagMask(void) const { return ~(lineMask() | blockMask()); }
	
	// Address decomposition
	inline offset_t offset(Address addr) const { return ot::mask(addr.offset()) & blockMask(); }
	inline set_t set(Address addr) const { return (ot::mask(addr.offset()) & lineMask()) >> blockBits(); }
	inline tag_t tag(Address addr) const { return ot::mask(addr.offset()) >> (blockBits() + rowBits()); }
	inline block_t block(Address addr) const { return ot::mask(addr.offset()) >> blockBits(); }
	inline Address round(Address addr) const
		{ if(addr.isNull()) return addr; else return Address(addr.page(), addr.offset() & ~(blockSize() - 1)); }
	
	// Modifiers
	void setAccessTime(int access_time);
	void setMissPenalty(int miss_penalty);
	void setBlockBits(int block_bits);
	void setRowBits(int set_bits);
	void setWayBits(int way_bits);
	void setReplacePolicy(replace_policy_t replace);
	void setWritePolicy(write_policy_t write);
	void setAllocate(bool allocate);
	void setWriteBufferSize(int write_buffer_size);
	void setReadPortSize(int read_port_size);
	void setWritePortSize(int write_port_size);

	// deprecated
	inline int rowBits(void) const { return _info.row_bits; }
	inline ot::mask lineMask(void) const { return (rowCount() - 1) << blockBits(); }
	inline ot::mask rowMask(void) const { return (rowCount() - 1) << blockBits(); }
	inline ot::mask line(const Address& addr) const { return (ot::mask(addr.offset()) & lineMask()) >> blockBits(); }
	inline int rowCount(void) const { return 1 << rowBits(); }
};

} } // otawa::hard

ENUM(otawa::hard::Cache::replace_policy_t);
ENUM(otawa::hard::Cache::write_policity_t);

#endif // OTAWA_HARD_CACHE_H
