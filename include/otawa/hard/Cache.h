/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hardware/Cache.h -- interface for Cache class.
 */
#ifndef OTAWA_HARDWARE_CACHE_H
#define OTAWA_HARDWARE_CACHE_H

#include <assert.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <otawa/base.h>

namespace otawa {
	
// Cache class
class Cache {
public:
	typedef enum replace_policy_t {
		NONE = 0,
		OTTHER = 1,
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
		int line_bits;
		int set_bits;
		replace_policy_t replace;
		write_policy_t write;
		bool allocate;
	} info_t;

private:
	info_t _info;
	const Cache *_next;

public:
	inline Cache(const info_t& info, const Cache *next = 0);
	inline Cache(const Cache& cache, const Cache *next = 0);
	
	// Simple accessors
	inline const Cache *nextLevel(void) const;
	inline size_t cacheSize(void) const;
	inline size_t blockSize(void) const;
	inline int wayCount(void) const;
	inline int lineCount(void) const;
	inline int setCount(void) const;
	inline replace_policy_t replacementPolicy(void) const;
	inline write_policy_t writePolicy(void) const;
	inline bool doesWriteAllocate(void) const;
	inline int missPenalty(void) const;
	
	// Low-level information
	inline int blockBits(void) const;
	inline int lineBits(void) const;
	inline int tagBits(void) const;	
	inline int setBits(void) const;
	inline mask_t blockMask(void) const; 
	inline mask_t lineMask(void) const;
	inline mask_t tagMask(void) const;
	
	// Address decomposition
	inline mask_t offset(address_t addr) const;
	inline mask_t line(address_t addr) const;
	inline mask_t tag(address_t addr) const;
	inline mask_t block(address_t addr) const;
};


// Inlines
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
	return 1 << (blockBits() + lineBits() + setBits());
}

inline size_t Cache::blockSize(void) const {
	return 1 << blockBits();
}

inline int Cache::setCount(void) const {
	return 1 << lineBits();
}

inline int Cache::lineCount(void) const {
	return 1 << lineBits();
}

inline int Cache::wayCount(void) const {
	return 1 << setBits();
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

inline int Cache::lineBits(void) const {
	return _info.line_bits;
}

inline int Cache::setBits(void) const {
	return _info.set_bits;
}

inline int Cache::tagBits(void) const {
	return 32 - blockBits() + lineBits();
}

inline mask_t Cache::blockMask(void) const {
	return blockSize() - 1;
}

inline mask_t Cache::lineMask(void) const {
	return (lineCount() - 1) << blockBits();
}

inline mask_t Cache::tagMask(void) const {
	return ~(lineMask() | blockMask());
}
	
inline mask_t Cache::offset(address_t addr) const {
	return ((mask_t)addr) & blockMask();
}

inline mask_t Cache::line(address_t addr) const {
	return (((mask_t)addr) & lineMask()) >> blockBits();
}

inline mask_t Cache::tag(address_t addr) const {
	return ((mask_t)addr) >> (blockBits() + lineBits());
}

inline mask_t Cache::block(address_t addr) const {
	return ((mask_t )addr) >> blockBits();
}

inline int Cache::missPenalty(void) const {
	return _info.miss_penalty;
}

} // otawa

#endif // OTAWA_HARDWARE_CACHE_H
