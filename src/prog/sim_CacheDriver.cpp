/*
 *	$Id$
 *	Copyright (c) 2007, IRIT-UPS <casse@irit.fr>
 *
 *	CacheDriver class implementation
 */

#include <otawa/sim/CacheDriver.h>

namespace otawa { namespace sim {

/**
 * @class CacheDriver
 * This class provides a simple interface to drive the cache management unit
 * of a simulator. To implement a specialized driver, you have just to
 * inherit from this class and override at least the @ref access() function.
 * @par
 * Many specialized driver are already provided in OTAWA. First, the
 * stateless always-hit / always-miss driver accessible by the static fields
 * @ref CacheDriver::ALWAYS_HIT and @ref CacheDriver::ALWAYS_MISS. There is
 * also an abstract driver with specialization for replacement and write
 * policies, @ref AbstractCacheDriver.
 */

/**
 * @enum CacheDriver::result_t
 * This enumeration allows to know the result of a cache access.
 */

/**
 * @var CacheDriver::result_t CacheDriver::MISS
 * The result of a cache access produces a miss.
 */

/**
 * @var CacheDriver::result_t CacheDriver::HIT
 * The result of a cache access produces a hit.
 */

/**
 * @enum CacheDriver::action_t
 * This enumeration identifies the kind of cache access.
 */

/**
 * @var CacheDriver::action_t CacheDriver::READ
 * A read is performed on the cache.
 */

/**
 * @var CacheDriver::action_t CacheDriver::WRITE
 * A write is performed on the cache.
 */


/**
 */ 
CacheDriver::~CacheDriver(void) {
}


/**
 * @fn result_t CacheDriver::access(address_t address, size_t size, action_t action);
 * This function is called each time the cache is accessed. It must be overload
 * to specialize the cache behavior.
 * @param address	Address of the accessed data.
 * @param size		Size of the accessed data.
 * @param action	Action performed.
 * @return			@ref HIT or @ref MISS according presence of the block of
 * 					data in the cache.
 */


// AlwaysHit class
class AlwaysHit: public CacheDriver {
public:
	result_t access(address_t address, size_t size, action_t action) {
		return HIT;
	} 	
};
static AlwaysHit HIT_DRIVER;


// AlwaysMiss class
class AlwaysMiss: public CacheDriver {
public:
	result_t access(address_t address, size_t size, action_t action) {
		return MISS;
	} 	
};
static AlwaysMiss MISS_DRIVER;


/**
 * This cache driver specialization generates a hit each time it is accessed.
 */
CacheDriver& CacheDriver::ALWAYS_HIT = HIT_DRIVER;


/**
 * This cache driver specialization generates a miss each time it is accessed.
 */
CacheDriver& CacheDriver::ALWAYS_MISS = MISS_DRIVER;

} } // otawa::sim
