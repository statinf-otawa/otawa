/*
 *	otawa::Bag class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_UTIL_BAG_H
#define OTAWA_UTIL_BAG_H

#include <elm/assert.h>
#include <elm/util/array.h>
#include <elm/genstruct/Vector.h>
#include <elm/util/Pair.h>

namespace otawa {

using namespace elm;

template <class T>
class Give {
public:
	inline Give(int c, T *a): cnt(c), arr(a) { }
	inline Give(genstruct::Vector<T>& v): cnt(v.length()), arr(v.detach()) { }
	inline int count(void) const { return cnt; }
	inline T *array(void) const { return arr; }
private:
	int cnt;
	T *arr;
};

template <class T>
class Bag {
public:

	// constructors
	inline Bag(void): cnt(0), arr(0) { }
	inline Bag(const Bag& bag) { copy(bag.cnt, bag.arr); }
	inline Bag(int c, const T *a) { copy(c, a); }
	inline Bag(int c, T *a) { copy(c, a); }
	inline Bag(const genstruct::Vector<T>& v) { copy(v); }
	inline Bag(Pair<int, T *> p) { copy(p.fst, p.snd); }
	inline Bag(const Give<T>& g): cnt(g.count()), arr(g.array()) { }
	inline ~Bag(void) { clear(); }

	// accessors
	inline bool isEmpty(void) const { return cnt == 0; }
	inline operator bool(void) const { return !isEmpty(); }
	inline int count(void) const { return cnt; }
	inline int size(void) const { return count(); }
	inline const T& get(int i) const { ASSERT(i >= 0 && i < cnt); return arr[i]; }
	inline T& get(int i) { ASSERT(i >= 0 && i < cnt); return arr[i]; }
	inline const T& operator[](int i) const { return get(i); }
	inline T& operator[](int i) { return get(i); }

	// mutators
	inline Bag& operator=(const Bag& bag) { clear(); copy(bag.cnt, bag.arr); return *this; }
	inline Bag& operator=(const genstruct::Vector<T>& v) { clear(); copy(v); return *this; }
	inline Bag& operator=(Pair<int, T *> p) { clear(); copy(p.fst, p.snd); return *this; }
	inline Bag& operator=(const Give<T>& g) { clear(); cnt = g.count(); arr = g.array(); return *this; }
	inline void clear(void) { if(arr) delete [] arr; }

private:
	inline void copy(int c, const T *a)
		{ cnt = c; arr = new T[c]; elm::array::copy(arr, a, c); }
	inline void copy(const genstruct::Vector<T>& v)
		{ cnt = v.length(); arr = new T[cnt]; for(int i = 0; i < cnt; i++) arr[i] = v[i]; }
	int cnt;
	T *arr;
};

};	// otawa

#endif	// OTAWA_UTIL_BAG_H
