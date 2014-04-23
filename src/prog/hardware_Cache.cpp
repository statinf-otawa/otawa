/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/Cache.cpp -- implementation of Cache class.
 */

#include <stdarg.h>
#include <otawa/hard/Cache.h>

using namespace elm;

namespace otawa { namespace hard {

/**
 * @class Cache
 * @ingroup hard
 * @p This class contains the configuration of a level of cache of processor.
 * @p It may represents direct-mapped or associative cache (with any associatvity
 * level) using bits as block/line boundaries. Supported management policies
 * (when they apply) are only LRU, FIFO, RANDOM and PLRU.
 * @p Once built, a cache object is viewed as a read-only database.
 */


/**
 */
Cache::Cache(void) {
}


/**
 */
Cache::Cache(const Cache& cache, const Cache *next): _info(cache._info), _next(next) {

}


/**
 */
Cache::~Cache(void) {
}


/**
 * @fn Cache::Cache(const info_t& info, const Cache *next);
 * Build a new cache with the given information.
 * @param info	Information about the cache.
 * @param next	Next level of cache (may be null).
 * @deprecated	Use setXXX() methods instead.
 */


/**
 * @fn Cache::Cache(const Cache& cache, const Cache *next);
 * Build a cache by cloning an existing cache and setting it in new cache
 * configuration.
 * @param next	Next level of cache (null if there is no more cache).
 */


/**
 * @fn Cache *Cache::nextLevel(void) const;
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
 * @fn int Cache::missPenalty(void) const;
 * Return the time penaly of a missed access to memory.
 * @return	Mise penalty in cycles.
 */

/**
 * @fn mask_t Cache::block(address_t addr) const;
 * Return the bits used for identifying uniquely a block, that is, tag and
 * line number.
 * @param addr	Address to take block from.
 * @return		Block part of the address.
 */


/**
 * @fn int Cache::writeBufferSize(void) const;
 * Get the write buffer size of this cache (0 for no write buffer).
 * @return	Write buffer size.
 */


/**
 * @fn int Cache::readPortSize(void) const;
 * Get the read port size of this cache (default to 1, must be non-null).
 * @return	Read port size.
 */


/**
 * @fn int Cache::writePortSize(void) const;
 * Get the write port size of this cache (default to 1 for data cache).
 * @return	Write port size.
 */


/**
 * @fn int Cache::wayBits(void) const;
 * Get the number of bits to count the ways in set associative cache. This value
 * is 1 for direct-mapped cache.
 * @return	Way count bits.	
 */


/**
 * @fn Address Cache::round(Address addr) const;
 * Round the given address to the address of the container cache block.
 * @param addr	Address to round.
 * @return		Round address.
 */


/**
 * Set the access time.
 * @param access_time	Access time in processor cycles.
 */
void Cache::setAccessTime(int access_time) {
	ASSERTP(access_time > 0, "bad access time");
	_info.access_time = access_time;
}


/**
 * Set the miss penaly time.
 * @param miss_penaly	Miss penalty time in processor cycles.
 */
void Cache::setMissPenalty(int miss_penalty) {
	ASSERTP(miss_penalty >= 0, "bad miss penalty");
	_info.miss_penalty = miss_penalty;
}


/***
 * Set the size of a cache block as a number of bits. The actucal size of
 * the block is 1 << block_bits.
 * @param block_bits	Bits to represent the block size.
 */
void Cache::setBlockBits(int block_bits) {
	ASSERTP(block_bits >= 0, "bad block bits");
	_info.block_bits = block_bits;
}


/**
 * Set the row count as a number of bits. The actual row count is
 * 1 << row_bits.
 * @param row_bits	Row count in bits.
 */
void Cache::setRowBits(int row_bits) {
	ASSERTP(row_bits >= 0, "bad row bits");
	_info.row_bits = row_bits;
}


/**
 * Set the way count as a number of bits. The actual way count is
 * 1 << way_bits.
 * @param way_bits	Way count in bits.
 */
void Cache::setWayBits(int way_bits) {
	ASSERTP(way_bits >= 0, "bad way bits");
	_info.way_bits = way_bits;
}


/**
 * Set the replace policy of the cache.
 * @param replace	Replace policy, one of OTHER, LRU, RANDOM, FIFO, PLRU.
 */
void Cache::setReplacePolicy(replace_policy_t replace) {
	ASSERTP(replace >= OTHER && replace <= PLRU, "bad replace policy");
	_info.replace = replace;
}


/**
 * Set the write policy of the cache.
 * @param write	Write policy, one of WRITE_THROUGH or WRITE_BACK.
 */
void Cache::setWritePolicy(write_policy_t write) {
	ASSERTP(write >= WRITE_THROUGH && write <= WRITE_BACK, "bad write policy");
	_info.write = write;
}


/**
 * Set the allocation state. If true, the write operation also allocate
 * a block. If false, no allocation arise.
 * @param allocate	Allocate state.
 */
void Cache::setAllocate(bool allocate) {
	_info.allocate = allocate;
}


/**
 * Set the write buffer size (0 for no write buffer).
 * @param write_buffer_size	Write buffer size.
 */
void Cache::setWriteBufferSize(int write_buffer_size) {
	ASSERTP(write_buffer_size >= 0, "bad write buffer size");
	_info.write_buffer_size = write_buffer_size;
}


/**
 * Set the read port size (at least 1).
 * @param read_port_size	Read port size.
 */
void Cache::setReadPortSize(int read_port_size) {
	ASSERTP(read_port_size >= 0, "bad read port size");
	_info.read_port_size = read_port_size;
}


/**
 * Set the write port size (at least 1).
 * @param read_write_size	Write port size.
 */
void Cache::setWritePortSize(int write_port_size) {
	ASSERTP(write_port_size >= 0, "bad write port size");
	_info.write_port_size = write_port_size;
}

} } // otawa::hard

// Serialization support
SERIALIZE(otawa::hard::Cache);

ENUM_BEGIN(otawa::hard::Cache::replace_policy_t)
	VALUE(otawa::hard::Cache::NONE),
	VALUE(otawa::hard::Cache::OTHER),
	VALUE(otawa::hard::Cache::LRU),
	VALUE(otawa::hard::Cache::RANDOM),
	VALUE(otawa::hard::Cache::FIFO),
	VALUE(otawa::hard::Cache::PLRU)
ENUM_END


ENUM_BEGIN(otawa::hard::Cache::write_policy_t)
	VALUE(otawa::hard::Cache::WRITE_THROUGH),
	VALUE(otawa::hard::Cache::WRITE_BACK)
ENUM_END
