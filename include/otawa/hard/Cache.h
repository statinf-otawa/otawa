/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hard/Cache.h -- interface for Cache class.
 */
#ifndef OTAWA_HARD_CACHE_H
#define OTAWA_HARD_CACHE_H

#include <assert.h>
#include <elm/io.h>
#include <elm/serial2/macros.h>
#include <elm/genstruct/Vector.h>
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
		PLRU = 5
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
	inline Cache(void);
	inline Cache(const Cache& cache, const Cache *next = 0);
	
	// Simple accessors
	inline const Cache *nextLevel(void) const;
	inline size_t cacheSize(void) const;
	inline size_t blockSize(void) const;
	inline int wayCount(void) const { return 1 << wayBits(); }
	inline int rowCount(void) const { return 1 << rowBits(); }
	inline int blockCount(void) const
		{ return 1 << (_info.row_bits + _info.way_bits); }
	inline replace_policy_t replacementPolicy(void) const;
	inline write_policy_t writePolicy(void) const;
	inline bool doesWriteAllocate(void) const;
	inline int missPenalty(void) const;
	inline int writeBufferSize(void) const { return _info.write_buffer_size; }
	inline int readPortSize(void) const { return _info.read_port_size; }
	inline int writePortSize(void) const { return _info.write_port_size; }
	
	// Low-level information
	inline int blockBits(void) const;
	inline int rowBits(void) const;
	inline int tagBits(void) const;	
	inline int wayBits(void) const;
	inline mask_t blockMask(void) const; 
	inline mask_t lineMask(void) const;
	inline mask_t tagMask(void) const;
	
	// Address decomposition
	inline mask_t offset(address_t addr) const;
	inline mask_t line(address_t addr) const;
	inline mask_t tag(address_t addr) const;
	inline mask_t block(address_t addr) const;
	
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
	
	// Deprecated
	inline Cache(const info_t& info, const Cache *next = 0);
};


// Inlines
inline Cache::Cache(void) {
}

inline Cache::Cache(const info_t& info, const Cache *next)
: _info(info), _next(next) {
}

inline Cache::Cache(const Cache& cache, const Cache *next)
: _info(cache._info), _next(next) {
}

inline const Cache *Cache::nextLevel(void) const {
	return _next;
}

inline size_t Cache::cacheSize(void) const {
	return 1 << (blockBits() + rowBits() + wayBits());
}

inline size_t Cache::blockSize(void) const {
	return 1 << blockBits();
}

inline Cache::replace_policy_t Cache::replacementPolicy(void) const {
	return _info.replace;
}

inline Cache::write_policy_t Cache::writePolicy(void) const {
	return _info.write;
}

inline bool Cache::doesWriteAllocate(void) const {
	return _info.allocate;
}

inline int Cache::blockBits(void) const {
	return _info.block_bits;
}

inline int Cache::rowBits(void) const {
	return _info.way_bits;
}

inline int Cache::wayBits(void) const {
	return _info.way_bits;
}

inline int Cache::tagBits(void) const {
	return 32 - blockBits() + rowBits();
}

inline mask_t Cache::blockMask(void) const {
	return blockSize() - 1;
}

inline mask_t Cache::lineMask(void) const {
	return (rowCount() - 1) << blockBits();
}

inline mask_t Cache::tagMask(void) const {
	return ~(lineMask() | blockMask());
}
	
inline mask_t Cache::offset(address_t addr) const {
	return ((mask_t)addr.address()) & blockMask();
}

inline mask_t Cache::line(address_t addr) const {
	return (((mask_t)addr.address()) & lineMask()) >> blockBits();
}

inline mask_t Cache::tag(address_t addr) const {
	return ((mask_t)addr.address()) >> (blockBits() + rowBits());
}

inline mask_t Cache::block(address_t addr) const {
	return ((mask_t )addr.address()) >> blockBits();
}

inline int Cache::missPenalty(void) const {
	return _info.miss_penalty;
}

} } // otawa::hard

ENUM(otawa::hard::Cache::replace_policy_t);
ENUM(otawa::hard::Cache::write_policity_t);

#endif // OTAWA_HARD_CACHE_H
