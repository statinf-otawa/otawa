/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hard/PureCache.h -- interface for Cache class.
 */
#ifndef OTAWA_HARD_PURECACHE_H
#define OTAWA_HARD_PURECACHE_H

#include <assert.h>
#include <elm/io.h>
#include <elm/serial2/macros.h>
#include <elm/genstruct/Vector.h>
#include <otawa/base.h>

namespace otawa { namespace hard {
	
// PureCache class
class PureCache {
public:
	typedef enum replace_policy_t {
		NONE = 0,
		OTHER = 1,
		LRU = 2,
		RANDOM = 3,
		FIFO = 4,
		PLRU = 5
	} replace_policy_t;
	
	int block_bits;
	int row_bits;
	int way_bits;
	replace_policy_t replace;
		
private:
	SERIALIZABLE(PureCache, 
		DFIELD(block_bits,4) &
		DFIELD(row_bits,12) &
		DFIELD(way_bits,0) & 	
		elm::field("replace", replace, LRU)
	);
	const PureCache *_next;

public:
	inline PureCache(void);
	virtual ~PureCache(void) { }
	
	// Simple accessors

	inline size_t cacheSize(void) const;
	inline size_t blockSize(void) const;
	inline int wayCount(void) const { return 1 << wayBits(); }
	inline int rowCount(void) const { return 1 << rowBits(); }
	inline int blockCount(void) const
		{ return 1 << (row_bits + way_bits); }
	inline replace_policy_t replacementPolicy(void) const;

	// Low-level information
	inline int blockBits(void) const;
	inline int rowBits(void) const;
	inline int tagBits(void) const;	
	inline int wayBits(void) const;
	inline t::mask blockMask(void) const;
	inline t::mask lineMask(void) const;
	inline t::mask tagMask(void) const;
	
	// Address decomposition
	inline t::mask offset(address_t addr) const;
	inline t::mask line(address_t addr) const;
	inline t::mask tag(address_t addr) const;
	inline t::mask block(address_t addr) const;
	
	// Modifiers
	void setBlockBits(int block_bits);
	void setRowBits(int set_bits);
	void setWayBits(int way_bits);
	void setReplacePolicy(replace_policy_t replace);

};


// Inlines
inline PureCache::PureCache(void) {
}

inline size_t PureCache::cacheSize(void) const {
	return 1 << (blockBits() + rowBits() + wayBits());
}

inline size_t PureCache::blockSize(void) const {
	return 1 << blockBits();
}

inline PureCache::replace_policy_t PureCache::replacementPolicy(void) const {
	return replace;
}


inline int PureCache::blockBits(void) const {
	return block_bits;
}

inline int PureCache::rowBits(void) const {
	return row_bits;
}

inline int PureCache::wayBits(void) const {
	return way_bits;
}

inline int PureCache::tagBits(void) const {
	return 32 - blockBits() + rowBits();
}

inline t::mask PureCache::blockMask(void) const {
	return blockSize() - 1;
}

inline t::mask PureCache::lineMask(void) const {
	return (rowCount() - 1) << blockBits();
}

inline t::mask PureCache::tagMask(void) const {
	return ~(lineMask() | blockMask());
}
	
inline t::mask PureCache::offset(address_t addr) const {
	return ((t::mask)addr.address()) & blockMask();
}

inline t::mask PureCache::line(address_t addr) const {
	return (((t::mask)addr.address()) & lineMask()) >> blockBits();
}

inline t::mask PureCache::tag(address_t addr) const {
	return ((t::mask)addr.address()) >> (blockBits() + rowBits());
}

inline t::mask PureCache::block(address_t addr) const {
	return ((t::mask)addr.address()) >> blockBits();
}



} } // otawa::hard

ENUM(otawa::hard::PureCache::replace_policy_t);

#endif 
