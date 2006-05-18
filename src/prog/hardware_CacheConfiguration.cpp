/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/hardware_CacheConfiguration.h -- CacheConfiguration class implementation.
 */

#include <assert.h>
#include <otawa/hard/CacheConfiguration.h>

namespace otawa { namespace hard {
	
/**
 * @class CacheConfiguration
 * This class represents the full configuration of caches of a processor.
 */


/**
 * Useful constant representing a configuration without cache.
 */
const CacheConfiguration CacheConfiguration::NO_CACHE;


/**
 * @fn CacheConfiguration::CacheConfiguration(const Cache *inst_cache, const Cache *data_cache);
 * Build a new configuration from the passed cache. For creating a no-cache
 * configuration, do not give any cache in argument. If you want to create
 * an unified cache, pass the the same cache as inst_cache and data_cache.
 * @param inst_cache	Instruction cache.
 * @param data_cache	Data cache.
 */


/**
 * @fn const Cache *CacheConfiguration::instCache(void) const;
 * Get the instruction cache.
 * @return	Instruction cache.
 */


/**
 * @fn const Cache *CacheConfiguration::dataCache(void) const;
 * Get the data cache.
 * @return	Data cache.
 */


/**
 * @fn bool CacheConfiguration::hasInstCache(void) const;
 * Check if some instruction is available (in a harvard or an unified
 * architecture).
 * @return	True if instruction cache is available, false else.
 */


/**
 * @fn bool CacheConfiguration::hasDataCache(void) const;
 * Check if some data is available (in a harvard or an unified architecture).
 * @return	True if a data cache data is available, false else.
 */


/**
 * @fn bool CacheConfiguration::isUnified(void) const;
 * Check if the cache ois unified.
 * @return True if the cache is unified, false else.
 */


/**
 * @fn bool CacheConfiguration::isHarvard(void) const;
 * Check if the cache follows the Harvard architecture.
 * @return True if the cache follows the Harvard architecture, false else.
 */

} } // otawa::hard
