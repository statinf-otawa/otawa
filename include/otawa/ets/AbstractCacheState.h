/*
 *	AbstractCacheState class interface
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
#ifndef OTAWA_ETS_ABSTRACTCACHESTATE_H
#define OTAWA_ETS_ABSTRACTCACHESTATE_H

#include <elm/util/BitVector.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/Vector.h>
#include <elm/io.h>
#include <otawa/base.h>

using namespace elm;

namespace otawa { namespace ets {

// AbstractCacheState class	
class AbstractCacheState{
	public :
		genstruct::Vector<BitVector *> cache_state;//<bitVector *>
		genstruct::HashTable<address_t, int> htable;//<l-block, its index in BitVector>
		int cache_line;
		typedef enum categorisation_t {
			ALWAYS_MISS = 0,
			ALWAYS_HIT = 1,
			FIRST_MISS = 2,
			CONFLICT = 3
		} categorisation_t;
		categorisation_t categorisation;
		genstruct::HashTable<address_t, int> hcat;//<l-block, its categorisation>
		
		inline AbstractCacheState(AbstractCacheState *acs);
		inline AbstractCacheState(int which_line);
		inline ~AbstractCacheState(void);
		
		bool areDifferent(AbstractCacheState *other);
		void join(AbstractCacheState *state1, AbstractCacheState *state2);
		void assignment(AbstractCacheState *other);
		bool byConflict();
};

// Inlines
inline AbstractCacheState::AbstractCacheState(AbstractCacheState *acs) {
	for(int i=0;i<acs->cache_state.length();i++)
		cache_state.add(new BitVector(*acs->cache_state[i]));
	htable.putAll(acs->htable);
	hcat.putAll(acs->hcat);
	cache_line = acs->cache_line;
}

inline AbstractCacheState::AbstractCacheState(int which_line) {
	cache_line = which_line;
}

inline AbstractCacheState::~AbstractCacheState(void) {
	for(int i=0;i<cache_state.length();i++)
		delete cache_state[i];
	cache_state.clear();
	htable.clear();
	hcat.clear();
}

} } // otawa::ets

#endif // OTAWA_ETS_ABSTRACTCACHESTATE_H
