/*
 *	BitSetManager class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#ifndef OTAWA_BITSETMANAGER_H_
#define OTAWA_BITSETMANAGER_H_

#include <elm/types.h>
#include <elm/assert.h>

using namespace elm;

class BitSetManager {
public:
	typedef t::uint32 word_t;
	typedef word_t *t;
	inline int size(void) const { return (n + 0x1f) >> 5; }
	inline word_t *top(t v) const { return v + size(); }
	inline BitSetManager(int _n): n(_n) { }
	inline t create(void) const { return new word_t[size()]; }
	inline void free(t v) const { delete [] v; }
	inline bool contains(t v, int i) const { return word(v, i) & mask(i); }
	inline void add(t v, int i) const { word(v, i) |= mask(i); }
	inline void remove(t v, int i) const { word(v, i) &= ~mask(i); }
	inline void empty(t d) const { while(d < top(d)) *d++ = 0; }
	inline void fill(t d) const { while(d < top(d)) *d++ = word_t(-1); }
	inline void copy(t d, t s) const { while(d < top(d)) *d++ = *s++; }
	inline void join(t d, t s) const { while(d < top(d)) *d++ |= *s++; }
	inline void meet(t d, t s) const { while(d < top(d)) *d++ &= *s++; }
	inline void diff(t d, t s) const { while(d < top(d)) *d++ &= ~*s++; }
	inline bool isEmpty(t v) const { while(v < top(v)) if(*v) return false; return true; }
	inline bool include(t d, t s) const { while(d < top(d)) { if((*d | *s) != *d) return false; d++; s++; } return true; }
	inline bool equals(t d, t s) const { while(d < top(d)) if(*d != *s) return false; return true; }
private:
	inline word_t& word(t v , int i) const { ASSERT(i >= 0 && i < n); return v[i >> 5]; }
	inline word_t mask(int i) const { return 1 << (i & 0x1f); }
	int n;
};

#endif /* BITSETMANAGER_H_ */
