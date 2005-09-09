/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/Cache.cpp -- implementation of Cache class.
 */

#include <stdarg.h>
#include <otawa/hardware/Cache.h>

namespace otawa {

/**
 * @class Cache
 * @p This class contains the configuration of a level of cache of processor.
 * @p It may represents direct-mapped or associative cache (with any associatvity
 * level) using bits as block/line boundaries. Supported management policies
 * (when they apply) are only LRU, FIFO, RANDOM and PLRU.
 * @p Once built, a cache object is viewed as a read-only database.
 */


/**
 * @fn Cache::Cache(const info_t& info, const Cache *next);
 * Build a new cache with the given information.
 * @param info	Information about the cache.
 * @param next	Next level of cache (may be null).
 */


/**
 * @fn Cache::Cache(const Cache& cache, const Cache *next);
 * Build a cache by cloning an existing cache and setting it in new cache
 * configuration.
 * @param next	Next level of cache (null if there is no more cache).
 */


/**
 * @fn Cache *Cache::nextLevel(void);
 * Get the next level of cache.
 * @return	Next cache level or null if there is no more cache.
 */


/**
 * @fn size_t Cache::cacheSize(void) const;
 * Get the cache size.
 * @return	Cache size in bytes.
 */


/**
 * @fn size_t Cache::blockSize(void) const;
 * Get the block size.
 * @return	Block size in bytes.
 */


/**
 * @fn int Cache::lineCount(void) const;
 * Get the count of lines.
 * @return	Count of lines.
 */


/**
 * @fn int Cache::setCount(void) const;
 * Get the count of sets.
 * @return	Count of sets.
 */


/**
 * @fn int Cache::wayCount(void) const;
 * Get the count of ways for associatives caches.
 * @return	Way count.
 */


/**
 * @fn Cache::replace_policy_t Cache::replacementPolicy(void) const;
 * Get the replacement policy.
 * @return	Replacement policy.
 */


/**
 * @fn Cache::write_policy_t Cache::writePolicy(void) const;
 * Get the write policy.
 * @return	Write policy.
 */


/**
 * @fn bool Cache::doesWriteAllocate(void) const;
 * Set the behaviour about write allocation.
 * @return	Write-allocate if true, No-write-allocate else.
 */


/**
 * @fn int Cache::blockBits(void) const;
 * Required bits count for a byte address in the block.
 * @return	Byte address bits in block.
 */


/**
 * @fn int Cache::lineBits(void) const;
 * Required bits count for a line index.
 * @return	Line index bits.
 */


/**
 * @fn int Cache::setBits(void) const;
 * Required bits count for a way index.
 * @return	Way index bits.
 */


/**
 * @fn int Cache::tagBits(void) const;
 * Required bits count in the tag part of the address.
 * @return	Tag bits.
 */


/**
 * @fn mask_t Cache::blockMask(void) const; 
 * Mask for selecting the address of a byte in a block in a memory address.
 * @return	Byte address mask in the block.
 */


/**
 * @fn mask_t Cache::lineMask(void) const;
 * Mask for selecting the line number in a memory address.
 * @return	Line number bits.
 */


/**
 * @fn mask_t Cache::tagMask(void) const;
 * Mask for selecting the tag in a memory address.
 * @return	Tag selection mask.
 */


/**
 * @fn mask_t Cache::offset(address_t addr) const;
 * Compute the offset of the accessed byte in the block from the memory address.
 * @param	addr	Memory address to compute from.
 * @return	Accessed byte offset in the block.
 */


/**
 * @fn mask_t Cache::line(address_t addr) const;
 * Compute the line number from the memory address.
 * @param	addr	Memory address to compute from.
 * @return	Line number.
 */


/**
 * @fn mask_t Cache::tag(address_t addr) const;
 * Compute the tag from the memory address.
 * @param	addr	Memory address to compute from.
 * @return	Tag.
 */


/**
 * @fn int Cache::missPenalty(void) const;
 * Return the time penaly of a missed access to memory.
 * @return	Mise penalty in cycles.
 */

} // otawa
