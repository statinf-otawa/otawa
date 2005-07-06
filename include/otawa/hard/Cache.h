/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hardware/Cache.h -- interface for Cache class.
 */
#ifndef OTAWA_HARDWARE_CACHE_H
#define OTAWA_HARDWARE_CACHE_H

#include <assert.h>
#include <elm/genstruct/Vector.h>
#include <otawa/base.h>

namespace otawa {
	
// Cache class
class Cache {
public:
	typedef enum replace_policy_t {
		NONE = 0,
		LRU = 1,
		RANDOM = 2,
		FIFO = 3
	} replace_policy_t;
	
	typedef enum write_policy_t {
		WRITE_THROUGH = 0,
		WRITE_BACK= 1
	} write_policity_t;
	
	typedef struct info_t {
		int level;
		int access_time;
		int miss_penalty;
		int block_bits;
		int line_bits;
		int way_bits;
		replace_policy_t replace;
		write_policy_t write;
		bool allocate;
	} info_t;

private:
	info_t _info;

public:
	inline Cache(const info_t& info);
	
	// Simple accessors
	inline int level(void) const;
	inline size_t cacheSize(void) const;
	inline size_t mapSize(void) const;
	inline size_t blockSize(void) const;
	inline int lineCount(void) const;
	inline int wayCount(void) const;
	inline replace_policy_t replacementPolicy(void) const;
	inline write_policy_t writePolicy(void) const;
	inline bool doesWriteAllocate(void) const;
	
	// Low-level information
	inline int blockBits(void) const;
	inline int lineBits(void) const;
	inline int wayBits(void) const;
	inline int mapBits(void) const;	
	inline mask_t blockMask(void) const; 
	inline mask_t lineMask(void) const;
	inline mask_t tagMask(void) const;
	
	// Address decomposition
	inline mask_t offset(address_t addr) const;
	inline mask_t line(address_t addr) const;
	inline mask_t tag(address_t addr) const;
};


// CacheHierarchy class
class CacheConfiguration: private elm::genstruct::Vector <const Cache *> {
public:

	// Constructors
	CacheConfiguration(const Cache *cache, ...);
	CacheConfiguration(int count, const Cache **caches);
	
	// Accessors
	inline int count(void) const;
	inline const Cache *get(int i) const;
};


// Inlines
inline Cache::Cache(const info_t& info): _info(info) {
}

inline int Cache::level(void) const {
	return _info.level;
}

inline size_t Cache::cacheSize(void) const {
	return 1 << (blockBits() + lineBits() + wayBits());
}

inline size_t Cache::mapSize(void) const {
	return 1 << mapBits();
}

inline size_t Cache::blockSize(void) const {
	return 1 << blockBits();
}

inline int Cache::lineCount(void) const {
	return 1 << lineBits();
}

inline int Cache::wayCount(void) const {
	return 1 << wayBits();
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

inline int Cache::wayBits(void) const {
	return _info.way_bits;
}

inline int Cache::mapBits(void) const {
	return blockBits() + lineBits();
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
	return ((mask_t)addr) >> mapBits();
}

inline int CacheConfiguration::count(void) const {
	return length();
}

inline const Cache *CacheConfiguration::get(int i) const {
	return elm::genstruct::Vector<const Cache *>::get(i);
}

} // otawa

#endif // OTAWA_HARDWARE_CACHE_H

