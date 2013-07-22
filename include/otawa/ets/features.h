/*
 *	ETS module
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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
#ifndef OTAWA_ETS_ETS_H
#define OTAWA_ETS_ETS_H

#include <otawa/prop/Identifier.h>
#include <otawa/proc/SilentFeature.h>

namespace otawa { namespace ets {

// Classes
class AbstractCacheState;

// features
extern SilentFeature ACS_FEATURE;
extern Identifier<AbstractCacheState *> ACS;

extern SilentFeature CACHE_FIRST_MISS_FEATURE;
extern Identifier<int> FIRST_MISSES;

extern SilentFeature CACHE_HIT_FEATURE;
extern Identifier<int> HITS;

extern SilentFeature CACHE_MISS_FEATURE;
extern Identifier<int> MISSES;

extern SilentFeature FLOWFACT_FEATURE;
extern Identifier<int> LOOP_COUNT;

extern SilentFeature BLOCK_TIME_FEATURE;
extern SilentFeature WCET_FEATURE;
extern Identifier<int> WCET;
extern Identifier<int> CONFLICTS;
		
} } // otawa::ets

#endif	// OTAWA_ETS_ETS_H
