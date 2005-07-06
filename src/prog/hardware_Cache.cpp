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
 * (when they apply) are only LRU, FIFO and RANDOM.
 * @p Once built, a cache object is viewed as a read-only database.
 */


/**
 * @fn Cache::Cache(const info_t& info);
 * Build a new cache with the given information.
 */


/**
 * @fn int Cache::level(void) const;
 * Get the cache level.
 * @return Cache level.
 */


/**
 * @fn size_t Cache::cacheSize(void) const;
 * Get the cache size.
 * @return	Cache size in bytes.
 */


/**
 * @fn size_t Cache::mapSize(void) const;
 * Get the size of the mapped part of the cache, that is, the size of memory
 * by the cache without associativity.
 * @return	Map size in bytes.
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
 * @fn int Cache::wayBits(void) const;
 * Required bits count for a way index.
 * @return	Way index bits.
 */


/**
 * @fn int Cache::mapBits(void) const;
 * Required bits count for a byte address in the map.
 * @return	Byte address bits in the map.
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
 * @class CacheConfiguration
 * This class represents the full configuration of caches of a processor.
 */


/**
 * Build a new cache configuration.
 * @param cache	First cache level.
 * @param ...	Other levels of cache (ended by NULL).
 */
CacheConfiguration::CacheConfiguration(const Cache *cache, ...) {
	va_list args;
	va_start(args, cache);
	while(cache) {
		add(cache);
		cache = va_arg(args, const Cache *);
	}
	va_end(args);
}


/**
 * Build a new cache configuration from given cache table.
 * @param count		Count of caches in the table.
 * @param caches	Cache table.
 */
CacheConfiguration::CacheConfiguration(int count, const Cache **caches) {
	assert((count && caches) || !count);
	for(int i = 0; i < count; i++)
		add(caches[i]);
}


/**
 * @fn int CacheConfiguration::count(void) const;
 * Get the count of levels in the cache configuration.
 * @return	Cache level count.
 */
 
 
/**
 * @fn const Cache *CacheConfiguration::get(int i) const;
 * Get the cache at the given level. It is an error to pass level greater
 * than the level count.
 * @param i		Level of cache (starts at 0, level 1 cache is got by a value of 0).
 * @return		Cache at the requested level.
 */

} // otawa
