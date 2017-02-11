/*
 *	categories module interface
 *	Copyright (c) 2012, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_CACHE_CATEGORIES_H_
#define OTAWA_CACHE_CATEGORIES_H_

#include <elm/assert.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>
#include "features.h"

#warning "This header is deprecated. Uses <otawa/cache/features.h> instead"

namespace otawa {

using namespace elm;
class Block;

// deprecation
typedef otawa::cache::category_t category_t;
const category_t INVALID_CATEGORY = otawa::cache::INVALID_CATEGORY;
const category_t ALWAYS_HIT = otawa::cache::ALWAYS_HIT;
const category_t FIRST_HIT = otawa::cache::FIRST_HIT;
const category_t FIRST_MISS = otawa::cache::FIRST_MISS;
const category_t ALWAYS_MISS = otawa::cache::ALWAYS_MISS;
const category_t NOT_CLASSIFIED = otawa::cache::NOT_CLASSIFIED;
extern Identifier<category_t>& CATEGORY;
extern Identifier<Block *>& CATEGORY_HEADER;
typedef cache::CategoryStats CategoryStats;
extern Identifier<CategoryStats *>& CATEGORY_STATS;

}	// otawa::cache

#endif // OTAWA_CACHE_FEATURES_H_
