/*
 *	$Id$
 *	cache module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_CACHE_FEATURES_H_
#define OTAWA_CACHE_FEATURES_H_

#include <elm/assert.h>
#include <otawa/cache/categorisation/CATBuilder.h>

namespace otawa {

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
extern Identifier<CategoryStats *> CATEGORY_STATS;

} // otawa

#endif // OTAWA_CACHE_FEATURES_H_
