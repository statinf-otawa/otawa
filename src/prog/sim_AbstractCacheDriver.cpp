/*
 *	$Id$
 *	Copyright (c) 2007, IRIT-UPS <casse@irit.fr>.
 *
 *	AbstractCacheDriver class implementation
 */

#include <otawa/sim/AbstractCacheDriver.h>
#include <otawa/hard/Cache.h>

namespace otawa { namespace sim {

/**
 * @class AbstractCacheDriver
 * This is an abstract implementation of the cache driver allowing to customize
 * only the cache replacement policy.
 * 
 * Implementations of this class includes:
 * @li @ref DirectMappedCacheDriver
 * @li @ref LRUCacheDriver
 * @li @ref FIFOCacheDriver
 */


/**
 * This method retrieve the cache driver matching the given cache description.
 * @param cache	Cache to look a driver for.
 * @return		Matching cache driver or null.
 */
CacheDriver *AbstractCacheDriver::lookup(const hard::Cache *cache) {
	if(cache->wayCount() == 1)
		return new DirectMappedCacheDriver(cache);
	else
		switch(cache->replacementPolicy()) {
		case hard::Cache::LRU:
			return new LRUCacheDriver(cache);
		case hard::Cache::FIFO:
			return new FIFOCacheDriver(cache);
		default:
			return 0;
		}
}


/**
 * @fn void AbstractCacheDriver::replace(tag_t tag, int num, tag_t *line);
 * This function is called each time a replacement in a set is required.
 * This method implementer must wipe out one of the block and replaced it
 * by the given tag.
 * @param tag	New tag to store in the set.
 * @param num	Number of the affected set/line.
 * @param line	Array base containing tags of the line.
 */


/**
 * @fn void AbstractCacheDriver::touch(int index, int num, tag_t *line);
 * This function is called each time a cache block in a set is accessed
 * (read / write). The implementer may use this call to update its tag
 * order in the set.
 * @param index	Index of the accessed tag in the tag array.
 * @param num	Number of the set/line accessed.
 * @param line	Base of the array containing tags of the current set/line.
 */ 


/**
 * Build a cache driver with the given cache.
 * @param cache	Used cache.
 */
AbstractCacheDriver::AbstractCacheDriver(const hard::Cache *cache)
: _cache(cache), lines(new tag_t[cache->blockCount()]) {
	ASSERT(cache);
	ASSERT(lines);
}


/**
 */
AbstractCacheDriver::~AbstractCacheDriver(void) {
	delete [] lines;
}


/**
 */ 
CacheDriver::result_t AbstractCacheDriver::access(
	address_t address,
	size_t size,
	action_t action)
{
	int line = _cache->line(address);

	tag_t *tags = lines + line*_cache->wayCount();
	tag_t tag = _cache->tag(address);

	for(int i = 0; i < 	_cache->wayCount(); i++)
		if(tags[i] == tag) {
			touch(i, line, tags);
			return HIT;
		}
	replace(tag, _cache->wayCount(), tags);
	return MISS;
}


/**
 * @class LRUCacheDriver
 * This class provide a cache driver implementing the LRU replacement policy.
 */


/**
 */
void LRUCacheDriver::touch(int index, int num, tag_t *line) {
	tag_t tag = line[index];
	for(int i = index; i > 0; i--)
		line[i] = line[i - 1];
	line[0] = tag;
}


/**
 */
void LRUCacheDriver::replace(tag_t tag, int num, tag_t *line) {
	for(int i = cache()->wayCount() - 1; i > 0 ; i--)
		line[i] = line[i - 1];
	line[0] = tag;
}


/**
 * Build a LRU cache driver.
 * @param cache	Used cache.
 */
LRUCacheDriver::LRUCacheDriver(const hard::Cache *cache)
: AbstractCacheDriver(cache) {
}


/**
 * @class DirectMappedCacheDriver
 * This class provide a cache driver implementing direct mapped cache driver.
 */


/**
 */
void DirectMappedCacheDriver::touch(int index, int num, tag_t *line) {
}


/**
 */
void DirectMappedCacheDriver::replace(tag_t tag, int num, tag_t *line) {
	line[0] = tag;
}


/**
 * Build a direct-mapped cache driver.
 * @param cache	Used cache.
 * @warning The passed cache must be direct-mapped.
 */
DirectMappedCacheDriver::DirectMappedCacheDriver(const hard::Cache *cache)
: AbstractCacheDriver(cache) {
	ASSERT(cache->wayCount() == 1);
}


/**
 * @class FIFOCacheDriver
 * This class provide a cache driver implementing a FIFO policy.
 */


/**
 */
void FIFOCacheDriver::touch(int index, int num, tag_t *line) {
}


/**
 */
void FIFOCacheDriver::replace(tag_t tag, int num, tag_t *line) {
	line[counters[num]] = tag;
	counters[num] = (counters[num] + 1) & (cache()->wayCount() - 1);
}


/**
 * Build an associative cache driver with FIFO replacement policy.
 * @param cache	Used cache.
 */
FIFOCacheDriver::FIFOCacheDriver(const hard::Cache *cache)
: AbstractCacheDriver(cache), counters(new int[cache->rowCount()]) {
	for(int i = 0; i < cache->rowCount(); i++)
		counters[i] = 0;
}


/**
 */
FIFOCacheDriver::~FIFOCacheDriver(void) {
	delete [] counters;
}

} } // otawa::sim
