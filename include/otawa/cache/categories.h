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

namespace otawa {

using namespace elm;
class BasicBlock;

namespace cache {

// categories
typedef enum category_t {
	INVALID_CATEGORY = 0,
	ALWAYS_HIT = 1,
	FIRST_HIT = 2,
	FIRST_MISS = 3,
	ALWAYS_MISS = 4,
	NOT_CLASSIFIED = 5
} category_t;
io::Output& operator<<(io::Output& out, category_t stats);


// category stats
class CategoryStats {
public:
	CategoryStats(void);
	void reset(void);
	inline void add(category_t cat)
		{ ASSERT(cat <= NOT_CLASSIFIED); counts[cat]++; _total++; }
	inline void addLinked(void) { _linked++; }

	inline int get(category_t cat) const
		{ ASSERT(cat <= NOT_CLASSIFIED); return counts[cat]; }
	inline int total(void) const { return _total; }
	inline int linked(void) const { return _linked; }
	
private:
	int counts[NOT_CLASSIFIED + 1];
	int _total, _linked;
};
io::Output& operator<<(io::Output& out, const CategoryStats& stats); 

// stats
extern Identifier<category_t> CATEGORY;
extern Identifier<BasicBlock *> CATEGORY_HEADER;
extern Identifier<CategoryStats *> CATEGORY_STATS;
extern p::feature ICACHE_CATEGORY_FEATURE;

}

// deprecation
typedef otawa::cache::category_t category_t;
const category_t INVALID_CATEGORY = otawa::cache::INVALID_CATEGORY;
const category_t ALWAYS_HIT = otawa::cache::ALWAYS_HIT;
const category_t FIRST_HIT = otawa::cache::FIRST_HIT;
const category_t FIRST_MISS = otawa::cache::FIRST_MISS;
const category_t ALWAYS_MISS = otawa::cache::ALWAYS_MISS;
const category_t NOT_CLASSIFIED = otawa::cache::NOT_CLASSIFIED;
extern Identifier<category_t>& CATEGORY;
extern Identifier<BasicBlock *>& CATEGORY_HEADER;
typedef cache::CategoryStats CategoryStats;
extern Identifier<CategoryStats *>& CATEGORY_STATS;

}	// otawa::cache

#endif // OTAWA_CACHE_FEATURES_H_
