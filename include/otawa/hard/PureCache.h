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
	inline mask_t blockMask(void) const; 
	inline mask_t lineMask(void) const;
	inline mask_t tagMask(void) const;
	
	// Address decomposition
	inline mask_t offset(address_t addr) const;
	inline mask_t line(address_t addr) const;
	inline mask_t tag(address_t addr) const;
	inline mask_t block(address_t addr) const;
	
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

inline mask_t PureCache::blockMask(void) const {
	return blockSize() - 1;
}

inline mask_t PureCache::lineMask(void) const {
	return (rowCount() - 1) << blockBits();
}

inline mask_t PureCache::tagMask(void) const {
	return ~(lineMask() | blockMask());
}
	
inline mask_t PureCache::offset(address_t addr) const {
	return ((mask_t)addr.address()) & blockMask();
}

inline mask_t PureCache::line(address_t addr) const {
	return (((mask_t)addr.address()) & lineMask()) >> blockBits();
}

inline mask_t PureCache::tag(address_t addr) const {
	return ((mask_t)addr.address()) >> (blockBits() + rowBits());
}

inline mask_t PureCache::block(address_t addr) const {
	return ((mask_t )addr.address()) >> blockBits();
}



} } // otawa::hard

ENUM(otawa::hard::PureCache::replace_policy_t);

#endif 
