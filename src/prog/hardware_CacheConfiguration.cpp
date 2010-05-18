/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/hardware_CacheConfiguration.h -- CacheConfiguration class implementation.
 */

#include <assert.h>
#include <otawa/hard/CacheConfiguration.h>
#include <elm/serial2/XOMUnserializer.h>

namespace otawa { namespace hard {
	
/**
 * @class CacheConfiguration
 * @ingroup hard
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


/**
 * Load a cache configuration from the given element.
 * @param element	Element to load from.
 * @return			Built cache configuration.
 */
CacheConfiguration *CacheConfiguration::load(elm::xom::Element *element) {
	elm::serial2::XOMUnserializer unserializer(element);
	CacheConfiguration *conf = new CacheConfiguration();
	try {
		unserializer >> *conf;
		return conf;
	}
	catch(elm::Exception& exn) {
		delete conf;
		throw exn;
	}
}


/**
 * Load a cache configuration from an XML file.
 * @param path	Path to the file.
 * @return		Built cache configuration.
 */
CacheConfiguration *CacheConfiguration::load(const elm::system::Path& path) {
	elm::serial2::XOMUnserializer unserializer(&path);
	CacheConfiguration *conf = new CacheConfiguration();
	try {
		unserializer >> *conf;
		return conf;
	}
	catch(elm::Exception& exn) {
		delete conf;
		throw exn;
	}
}

/**
 * Compute name of the cache.
 * @param cache		Cache to get name for.
 * @return			Cache name.
 */
string CacheConfiguration::cacheName(const Cache *cache) const {
	int level = 1;

	// traverse cache hierarchy
	const Cache *icache = instCache(), *dcache = dataCache();
	while(icache != cache && dcache != cache) {
		if(!icache && !dcache)
			return "unknown";
		if(icache)
			icache = icache->nextLevel();
		if(dcache)
			dcache = dcache->nextLevel();
	}

	// build the name
	StringBuffer buf;
	buf << 'L' << level;
	if(!icache)
		buf << " data";
	else if(!dcache)
		buf << " instruction";
	else
		buf << " unified";
	buf << " cache";
	return buf.toString();
}


} } // otawa::hard

SERIALIZE(otawa::hard::CacheConfiguration)
