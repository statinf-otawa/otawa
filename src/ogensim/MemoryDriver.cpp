/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/gensim/MemoryDriver.h>

namespace otawa {
namespace sim {

/**
 * @class MemoryDriver
 * This class provides a simple interface to drive the cache management unit
 * of a simulator. To implement a specialized driver, you have just to
 * inherit from this class and override at least the @ref access() function.
 * @par
 * Many specialized driver are already provided in OTAWA. First, the
 * stateless always-hit / always-miss driver accessible by the static fields
 * @ref MemoryDriver::ALWAYS_HIT and @ref MemoryDriver::ALWAYS_MISS. There is
 * also an abstract driver with specialization for replacement and write
 * policies, @ref AbstractMemoryDriver.
 */

/**
 * @enum MemoryDriver::result_t
 * This enumeration allows to know the result of a cache access.
 */

/**
 * @var MemoryDriver::location_t MemoryDriver::DATA_CACHE
 * The address is in the data cache.
 */

/**
 * @var MemoryDriver::location_t MemoryDriver::DATA_SPM
 * The address is in the data scratchpad memory.
 */

/**
 * @enum MemoryDriver::location_t
 * This enumeration identifies the location of an address.
 */

/**
 */
MemoryDriver::~MemoryDriver(void) {
}

/**
 * @fn location_t MemoryDriver::locate(address_t address);
 * This function is called to locate an address in the memory system.
 * @param address       Address of the accessed data.
 * @return                      @ref DATA_CACHE or @ref DATA_SPM 
 */

// AlwaysDataCache class
class AlwaysDataCache : public MemoryDriver {
public:
	location_t locate(address_t address) {
		return DATA_CACHE;
	}
};
static AlwaysDataCache ALWAYS_DATA_CACHE_DRIVER;

/**
 * This memory driver specialization return DATA_CACHE each time it is accessed.
 */
MemoryDriver& MemoryDriver::ALWAYS_DATA_CACHE = ALWAYS_DATA_CACHE_DRIVER;

}
} // otawa::sim
