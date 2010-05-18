/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hard/CacheConfiguration.h -- CacheConfiguration class interface.
 */
#ifndef OTAWA_HARD_CONFIGURATION_CACHE_H
#define OTAWA_HARD_CONFIGURATION_CACHE_H

#include <otawa/hard/Cache.h>
#include <elm/system/Path.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa { namespace hard {

// CacheConfiguration class
class CacheConfiguration {
	SERIALIZABLE(CacheConfiguration, FIELD(icache) & FIELD(dcache));
	const Cache *icache, *dcache;
public:
	virtual ~CacheConfiguration(void) { }
	static const CacheConfiguration NO_CACHE;
	inline CacheConfiguration(const Cache *inst_cache = 0,
		const Cache *data_cache = 0);
	inline const Cache *instCache(void) const;
	inline const Cache *dataCache(void) const;
	inline bool hasInstCache(void) const;
	inline bool hasDataCache(void) const;
	inline bool isUnified(void) const;
	inline bool isHarvard(void) const;
	static CacheConfiguration *load(elm::xom::Element *element);
	static CacheConfiguration *load(const elm::system::Path& path);
	string cacheName(const Cache *cache) const;
};

// Inlines
inline CacheConfiguration::CacheConfiguration(const Cache *inst_cache,
const Cache *data_cache): icache(inst_cache), dcache(data_cache) {
}

inline const Cache *CacheConfiguration::instCache(void) const {
	return icache;
}

inline const Cache *CacheConfiguration::dataCache(void) const {
	return dcache;
}

inline bool CacheConfiguration::hasInstCache(void) const {
	return icache != 0;
}

inline bool CacheConfiguration::hasDataCache(void) const {
	return dcache != 0;
}

inline bool CacheConfiguration::isUnified(void) const {
	return icache == dcache;
}

inline bool CacheConfiguration::isHarvard(void) const {
	return icache != dcache;
}

} } // otawa::hard

#endif	// OTAWA_HARD_CONFIGURATION_CACHE_H
