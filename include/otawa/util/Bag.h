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

#include <elm/array.h>
#include <elm/assert.h>
#include <elm/data/Vector.h>
#include <elm/util/Pair.h>

namespace otawa {

using namespace elm;

template <class T>
class Give {
public:
	inline Give(int c, T *a): cnt(c), arr(a) { }
	inline Give(Vector<T>& v): cnt(v.length()), arr(v.detach().buffer()) { }
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
	inline Bag(int c): cnt(c), arr(new T[c]) { }
	inline Bag(const Bag& bag) { copy(bag.cnt, bag.arr); }
	inline Bag(int c, const T *a): cnt(0), arr(nullptr) { copy(c, a); }
	inline Bag(int c, T *a): cnt(0), arr(nullptr) { copy(c, a); }
	inline Bag(const Vector<T>& v): cnt(0), arr(nullptr) { copy(v); }
	inline Bag(Pair<int, T *> p): arr(nullptr), cnt(0) { copy(p.fst, p.snd); }
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

	// Iter class
	class Iter: public PreIterator<Iter, T> {
	public:
		inline Iter(): b(nullptr), i(0) { }
		inline Iter(const Bag<T>& bag): b(&bag), i(0) { }
		inline Iter(const Bag<T>& bag, int index): b(&bag), i(index) { }
		inline bool ended(void) const { return i >= b->count(); }
		inline const T& item(void) const { return (*b)[i]; }
		inline void next(void) { i++; }
		inline bool operator==(const Iter& it) { return b == it.b && i == it.i; }
		inline bool operator!=(const Iter& it) { return !operator==(it); }
	private:
		const Bag<T> *b;
		int i;
	};
	inline Iter items(void) const { return Iter(*this); }
	inline Iter operator*(void) const { return items(); }
	inline Iter begin(void) const { return Iter(*this); }
	inline Iter end(void) const { return Iter(*this, count()); }

	// mutators
	inline void set(const Bag& bag)				{ clear(); copy(bag.cnt, bag.arr); }
	inline void set(const Vector<T>& v)			{ clear(); copy(v); }
	inline void set(Pair<int, T *> p)			{ clear(); copy(p.fst, p.snd); }
	inline void set(const Give<T>& g)			{ clear(); cnt = g.count(); arr = g.array(); }
	inline void give(Bag& bag)					{ clear(); cnt = bag.cnt, arr =  bag.arr; bag.cnt = 0; bag.arr = 0; }

	inline void give(Vector<T>& g)				{ clear(); cnt = g.count(); arr = g.detach().buffer(); }
	inline void give(Pair<int, T *> p)			{ clear(); cnt = p.fst; arr = p.snd; }
	inline void clear(void)						{ if(arr) delete [] arr; }

	inline Bag& operator=(const Bag& bag)		{ set(bag); return *this; }
	inline Bag& operator=(const Vector<T>& v)	{ set(v); return *this; }
	inline Bag& operator=(Pair<int, T *> p)		{ set(p); return *this; }
	inline Bag& operator=(const Give<T>& g)		{ set(g); return *this; }

	inline Bag& operator<<(Bag& bag)			{ give(bag); return *this; }
	inline Bag& operator<<(Vector<T>& v)		{ give(v); return *this; }
	inline Bag& operator<<(Pair<int, T *> p)	{ give(p); return *this; }
	inline Bag& operator<<(const Give<T>& g)	{ give(g); return *this; }

private:
	inline void copy(int c, const T *a)
		{ cnt = c; arr = new T[c]; elm::array::copy(arr, a, c); }
	inline void copy(const Vector<T>& v)
		{ cnt = v.length(); arr = new T[cnt]; for(int i = 0; i < cnt; i++) arr[i] = v[i]; }
	int cnt;
	T *arr;
};

template <class T>
Bag<T> move(Vector<T>& v) { return Bag<T>(Give<T>(v)); }
template <class T>
Bag<T> move(int c, T *t) { return Bag<T>(Give<T>(c, t)); }

};	// otawa

#endif	// OTAWA_UTIL_BAG_H
