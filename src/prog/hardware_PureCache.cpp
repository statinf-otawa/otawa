
#include <stdarg.h>
#include <otawa/hard/PureCache.h>

using namespace elm;

namespace otawa { namespace hard {

/**
 * @class PureCache
 * @ingroup hard
 */

/**
 * @fn Cache::Cache(const Cache& cache, const Cache *next);
 * Build a cache by cloning an existing cache and setting it in new cache
 * configuration.
 * @param next	Next level of cache (null if there is no more cache).
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
 * @fn int Cache::rowCount(void) const;
 * Get the count of rows.
 * @return	Count of rows.
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
 * @fn int Cache::blockBits(void) const;
 * Required bits count for a byte address in the block.
 * @return	Byte address bits in block.
 */


/**
 * @fn int Cache::rowBits(void) const;
 * Required bits count for a row index.
 * @return	Line index bits.
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
 * @fn mask_t Cache::block(address_t addr) const;
 * Return the bits used for identifying uniquely a block, that is, tag and
 * line number.
 * @param addr	Address to take block from.
 * @return		Block part of the address.
 */



/**
 * @fn int Cache::wayBits(void) const;
 * Get the number of bits to count the ways in set associative cache. This value
 * is 1 for direct-mapped cache.
 * @return	Way count bits.	
 */



/***
 * Set the size of a cache block as a number of bits. The actucal size of
 * the block is 1 << block_bits.
 * @param block_bits	Bits to represent the block size.
 */
void PureCache::setBlockBits(int block_bits) {
	ASSERTP(block_bits >= 0, "bad block bits");
	block_bits = block_bits;
}


/**
 * Set the row count as a number of bits. The actual row count is
 * 1 << row_bits.
 * @param row_bits	Row count in bits.
 */
void PureCache::setRowBits(int row_bits) {
	ASSERTP(row_bits >= 0, "bad row bits");
	row_bits = row_bits;
}


/**
 * Set the way count as a number of bits. The actual way count is
 * 1 << way_bits.
 * @param way_bits	Way count in bits.
 */
void PureCache::setWayBits(int way_bits) {
	ASSERTP(way_bits >= 0, "bad way bits");
	way_bits = way_bits;
}


/**
 * Set the replace policy of the cache.
 * @param replace	Replace policy, one of OTHER, LRU, RANDOM, FIFO, PLRU.
 */
void PureCache::setReplacePolicy(replace_policy_t replace) {
	ASSERTP(replace >= OTHER && replace <= PLRU, "bad replace policy");
	replace = replace;
}


} } // otawa::hard

// Serialization support
SERIALIZE(otawa::hard::PureCache);

ENUM_BEGIN(otawa::hard::PureCache::replace_policy_t)
	VALUE(otawa::hard::PureCache::NONE),
	VALUE(otawa::hard::PureCache::OTHER),
	VALUE(otawa::hard::PureCache::LRU),
	VALUE(otawa::hard::PureCache::RANDOM),
	VALUE(otawa::hard::PureCache::FIFO),
	VALUE(otawa::hard::PureCache::PLRU)
ENUM_END

