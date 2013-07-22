/*
 *	AbstractCacheState class implementation
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
#include <otawa/ets/AbstractCacheState.h>

#define ACS_OUT(txt) 	// txt

namespace otawa { namespace ets {

/**
 * @class AbstractCacheState
 * This class is used to represent an abstract cache line state.
 */
 
/** 
 * @var AbstractCacheState::cache_state;
 * Represent the cache state (table of set) with all l-blocks being in this line.
 */

/** 
 * @var AbstractCacheState::htable;
 * <l-block, its index in cache_state>
 */
 
/** 
 * @var AbstractCacheState::cache_line;
 * It is the number of the cache line.
 */

/** 
 * @var AbstractCacheState::categorisation;
 * Categorisation (a_h, a_m, f_m, c) according to F. Mueller in 1994.
 */

/** 
 * @var AbstractCacheState::hcat;
 * <l-block, its categorisation>
 */

/** 
 * @fn AbstractCacheState::AbstractCacheState(AbstractCacheState *acs);
 * Build a new ACS with an other: acs.
 * @param acs	AbstractCacheState to assign.
 */
 
/** 
 * @fn AbstractCacheState::AbstractCacheState(int which_line);
 * Build a new ACS with the number of the cache line.
 * @param which_line	The cache line to simulate.
 */
 
/** 
 * @fn bool AbstractCacheState::areDifferent(AbstractCacheState *other);
 * Return true if other is not equal whith this, else return false.
 * @param other	AbstractCacheState to compare.
 */
bool AbstractCacheState::areDifferent(AbstractCacheState *other){
	bool neq = false;
	int i = 0;
	while ((i<other->cache_state.length())&&(!neq)){
		neq = !(cache_state[i]->equals(*other->cache_state[i]));
		i++;
	}
	return neq;
}
 
/**
 * @fn void AbstractCacheState::join(AbstractCacheState *state1, AbstractCacheState *state2);
 * Join state1 and state2 to this, according to C. Ferdinand (Join - Must).
 * @param state1	AbstractCacheState to merge.
 * @param state2	AbstractCacheState to merge.
 * @remarks Ferdinand's join function in the Must analysis:
 * @f[
 * 		I(\hat{c}_1,\hat{c}_2) = \hat{c}, where 
 * 			 \indent \hat{c}(l_x)\ =\ \{s_i\ |\ \exists\ l_a,\ l_b\ with\ s_i\ \in\ \hat{c}_1(l_a),\ s_i\ \in\ \hat{c}_2(l_b)\ and\ x\ =\ max(a,\ b)\}
 * @f]
 */
void AbstractCacheState::join(AbstractCacheState *state1, AbstractCacheState *state2){
	int a, b;
	for(int j=0;j<cache_state.length();j++)
		cache_state[j]->clear();
		
	//Adding of state1->hcat and state2->hcat in this.
	for(genstruct::HashTable<address_t, int>::KeyIterator key(state1->hcat); key; key++)
		if(!hcat.exists(key))
			hcat.put(key, state1->hcat.get(key, -1));
	for(genstruct::HashTable<address_t, int>::KeyIterator yek(state2->hcat); yek; yek++)
		if(!hcat.exists(yek))
			hcat.put(yek, state1->hcat.get(yek, -1));
		
	//Ferdinand's Algorithme (Intersection(Join) - Must).
	for(genstruct::HashTable<address_t, int>::KeyIterator k(htable); k; k++){
		a=-1;
		b=-1;
		for(int i=0;i<state1->cache_state.length();i++){
			if(state1->cache_state[i]->bit(state1->htable.get(k, -1)))
				a=i;
			if(state2->cache_state[i]->bit(state2->htable.get(k, -1)))
				b=i;
		}
		if((a!=-1)&&(b!=-1))
			if(a < b)
				cache_state[b]->set(k,true);
			else
				cache_state[a]->set(k,true);
	}
	
}
		
/**
 * @fn void AbstractCacheState::assignment(AbstractCacheState *other);
 * A simple assignment from other to this.
 * @param other	AbstractCacheState to assign.
 */
void AbstractCacheState::assignment(AbstractCacheState *other){
	for(int i=0;i<other->cache_state.length();i++){
		cache_state[i]->clear();
		cache_state[i]->applyOr(*other->cache_state[i]);
	}
	for(genstruct::HashTable<address_t, int>::KeyIterator key(other->htable); key; key++){
		if(!htable.exists(key))
			htable.put(key, other->htable.get(key, -1));
	}
	for(genstruct::HashTable<address_t, int>::KeyIterator k(other->hcat); k; k++){
		if(!hcat.exists(k))
			hcat.put(k, other->hcat.get(k, -1));
	}
	cache_line = other->cache_line;
}

/**
 * @fn bool AbstractCacheState::byConflict();
 * Look if it is conflict, that is to say if they are many true bit of each set.
 */
bool AbstractCacheState::byConflict(){
	int count_conflict = 0;
	int j = 0;
	bool conflict = false;
	while((j<cache_state.length())&&(!conflict)){
		for(int i=0;i<cache_state[j]->size();i++)
			if(cache_state[j]->bit(i))
				count_conflict++;
		if (count_conflict > 1)
			conflict = true;
		count_conflict = 0;
		j++;
	}
	return conflict;
}
		
} }// otawa::ets
