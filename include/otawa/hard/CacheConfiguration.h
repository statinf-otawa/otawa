/*
 *	$Id$
 *	CacheConfiguration class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-10, IRIT UPS.
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
#ifndef OTAWA_HARD_CONFIGURATION_CACHE_H
#define OTAWA_HARD_CONFIGURATION_CACHE_H

#include <elm/sys/Path.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/features.h>
#include <otawa/proc/Accessor.h>
#include <otawa/proc/Feature.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa { namespace hard {

// CacheConfiguration class
class CacheConfiguration {
	SERIALIZABLE(CacheConfiguration, FIELD(icache) & FIELD(dcache));
public:
	static const CacheConfiguration NO_CACHE;
	static CacheConfiguration *load(elm::xom::Element *element);
	static CacheConfiguration *load(const elm::sys::Path& path);

	CacheConfiguration(const Cache *inst_cache = 0, const Cache *data_cache = 0);
	virtual ~CacheConfiguration(void);

	inline const Cache *instCache(void) const { return icache; }
	inline const Cache *dataCache(void) const  { return dcache; }
	inline bool hasInstCache(void) const { return icache != 0; }
	inline bool hasDataCache(void) const { return dcache != 0; }
	inline bool isUnified(void) const { return icache == dcache; }
	inline bool isHarvard(void) const { return icache != dcache; }
	string cacheName(const Cache *cache) const;

protected:
	const Cache *icache, *dcache;

};


// features
extern FunAccessor<const Cache *> L1_ICACHE;
extern FunAccessor<const Cache *> L1_DCACHE;

} } // otawa::hard

#endif	// OTAWA_HARD_CONFIGURATION_CACHE_H
