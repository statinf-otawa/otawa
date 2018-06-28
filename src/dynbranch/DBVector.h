/*
 *	$Id$
 *	Vector class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2004-08, IRIT UPS.
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
#ifndef DBVECTOR
#define DBVECTOR

#include <elm/assert.h>
#include <elm/data/Array.h>
#include <elm/data/Vector.h>
#include <elm/PreIterator.h>
#include <iostream>
#include <typeinfo>
#include <elm/alloc/DefaultAllocator.h>
#include <elm/log/Log.h>

using namespace elm;

namespace otawa {

namespace dynbranch {

#define SHOWSHOW
// EmbedVector class
template <class T>
class Vector {
public:
// 	they are used to create Vector, but no the T
//	inline void* operator new(std::size_t sz) { static int i = 0; i++; elm::cout << "vector.new1 [" << i << "] " << sz << io::endl; return ::operator new(sz);}
//	inline void* operator new[](std::size_t sz) { static int i = 0; i++; elm::cout << "vector.new2 [" << i << "] " << sz << '\n'; return ::operator new(sz); }
//	inline void* operator new(size_t size, void* ptr) { static int i = 0; i++; elm::cout << "vector.new3 [" << i << "] " << size << io::endl; return ::operator new(size, ptr); } // placement new
//	inline void* operator new[](size_t size, void* ptr) { static int i = 0; i++; elm::cout << "vector.new4 [" << i << "] " << size << io::endl; return ::operator new(size, ptr); } // placement new
//	template<class G> inline void *operator new(size_t size, G& alloc) { static int i = 0; i++; elm::cout << "vector.new5 [" << i << "] " << io::endl; return alloc.template allocate(size); }
//	template<class G> inline void *operator new[](size_t size, G& alloc) { static int i = 0; i++; elm::cout << "vector.new6 [" << i << "] " << size << io::endl; return alloc.template allocate(size); }
//

	inline bool collect() const {
		if(!gc)
			return true;

		if(tab == 0 && cap != 0) {
			ASSERT(0);
		}

		if(tab == 0 || cap == 0)
			return true;

		bool already = gc->mark(tab, sizeof(T)*cap);

		return already;
	}

	inline Vector(elm::DefaultAllocator* _g, int _cap): gc(_g), tab(0), cap(_cap), cnt(0) {
		if(gc == 0) {
			static int i = 0;
			i++;
			//if(i == 5) assert(0); // see if there is anymore potential value created without gc
		}
//		else {
//			elm::cout << "gc = " << (void*)gc << io::endl;
//		}


		countX++;
		if(cap == 0)
			return;
		if(gc == 0)
			tab = (new T[_cap]);
		else {
			ASSERT(0); // just to see what comes here
			tab = static_cast<T*>(gc->allocate(sizeof(T)*_cap));
		}

	}

	inline Vector(DefaultAllocator* _g, const Vector<T>& vec): gc(_g), tab(0), cap(0), cnt(0) { // copy constructor with the uses of GC
		countX++;
		copy(vec);
	}

	inline Vector(int _cap = 8): gc(0), tab(new T[_cap]), cap(_cap), cnt(0) { 	}

	inline Vector(const Vector<T>& vec): gc(vec.gc), tab(0), cap(0), cnt(0) { // copy constructor
		if(gc != 0)
			ASSERT(0);
		copy(vec);
	}

	inline ~Vector(void) {
		if(!gc && tab) delete [] tab;
	}

	T* getTab(void) const { return tab; }

	// Iterator
	class Iterator: public PreIterator<Iterator, const T&> {
	public:
		friend class Vector;
		inline Iterator(const Vector& vec): _vec(vec), i(0) { }
		inline Iterator(const Iterator& iter): _vec(iter._vec), i(iter.i) { }
		inline bool ended(void) const { return i >= _vec.length(); }
		inline const T& item(void) const { return _vec[i]; }
		inline void next(void) { i++; }
		inline int index(void) { return i; }
		const Vector<T>& vector(void) { return _vec; }
	private:
		const Vector<T>& _vec;
		int i;
	};

	// MutableIterator
	class MutableIterator: public PreIterator<MutableIterator, T&> {
	public:
		friend class Vector;
		inline MutableIterator(Vector& vec): _vec(vec), i(0) { }
		inline MutableIterator(const MutableIterator& iter): _vec(iter._vec), i(iter.i) { }
		inline bool ended(void) const { return i >= _vec.length(); }
		inline T& item(void) const { return _vec[i]; }
		inline void next(void) { i++; }
		inline int index(void) { return i; }
		Vector<T>& vector(void) { return _vec; }
	private:
		Vector<T>& _vec;
		int i;
	};

	// Accessors
	int count(void) const { return cnt; }
	inline int length(void) const { return count(); }
	inline int capacity(void) const;
	inline bool isEmpty(void) const { return cnt == 0; }
	inline const T& get(int index) const;
	inline T& item(int index);
	inline void set(int index, const T& value);
	inline T& operator[](int index) { return item(index); }
	inline const T& operator[](int index) const { return get(index); }
	bool contains(const T& value) const;
	template <template <class _> class C> inline bool containsAll(const C<T>& items)
		{ for(typename C<T>::Iterator item(items); item; item++)
			if(!contains(item)) return false; return true; }
	int indexOf(const T& value, int start = 0) const;
	int lastIndexOf(const T& value, int start = -1) const;
	inline operator bool(void) const { return cnt != 0; }
	inline bool operator==(const Vector<T>& v) const
		{ if(length() != v.length()) return false; for(int i = 0; i < length(); i++) if(get(i) != v[i]) return false; return true; }
	inline bool operator!=(const Vector<T>& v) const { return !(*this == v); }
	inline const T& first(void) const
		{ ASSERT(cnt > 0); return tab[0]; }
	inline const T& last(void) const
		{ ASSERT(cnt > 0); return tab[cnt - 1]; }
	inline Iterator find(const T &item)
		{ Iterator i(*this); while(i && *i != item) i++; return i; }
	inline Iterator find (const T &item, const Iterator &start)
		{ Iterator i(start); while(i && *i != item) i++; return i; }
	inline const T& top(void) const
		{ ASSERTP(cnt > 0, "no more data in the stack"); return tab[cnt - 1]; }
	inline T& top(void)
		{ ASSERTP(cnt > 0, "no more data in the stack"); return tab[cnt - 1]; }

	// Mutators
	inline void add(void);
	inline void add(const T& value);
	template <template <class _> class C> inline void addAll(const C<T>& items)
		{ for(typename C<T>::Iterator item(items); item; item++) add(item); }
	void removeAt(int index);
	inline void remove(const T& value) { int i = indexOf(value); if(i >= 0) removeAt(i); }
	inline void remove(const Iterator& iter) { removeAt(iter.i); }
	inline void remove(const MutableIterator& iter) { removeAt(iter.i); }
	template <template <class _> class C> inline void removeAll(const C<T>& items)
		{ for(typename C<T>::Iterator item(items); item; item++) remove(item); }
	void insert(int index, const T& value);
	inline void clear(void) { cnt = 0; }
	void grow(int new_cap);
	void setLength(int new_length);
	inline Array<T> detach(void);
	inline void copy(const Vector& vec);
	inline Vector<T>& operator=(const Vector& vec) { copy(vec); return *this; };
	inline void swallow(Vector<T>& v) { if(tab) delete [] tab; tab = v.tab; v.tab = 0; }
	inline void insert(const T &item) { add(item); }
	inline void push(const T& value)
		{ add(value); }
	inline const T pop(void)
		{ ASSERTP(cnt > 0, "no more data to pop"); return tab[--cnt]; }
	inline void addFirst(const T &item) { insert(0, item); }
	inline void addLast(const T &item) { add(item); }
	inline void removeFirst(void) { removeAt(0); }
	inline void removeLast(void) { removeAt(cnt - 1); }
	inline void set (const Iterator &pos, const T &item) { tab[pos.pos] = item; }
	inline void addAfter(const Iterator &pos, const T &item) { insert(pos.i + 1, item); }
	inline void addBefore(const Iterator &pos, const T &item) { insert(pos.i, item); }

	// debugging
	static unsigned long countX, countY, maxUse, minUse;
	inline void checkState(void* x) {
		//elm::cout << __SOURCE_INFO__ << "tab @ " << (void*)tab << ", gc = " << (void*)gc << io::endl;
		ASSERT(gc == x);
	}
	inline void checkState(void* x) const {
		//elm::cout << __SOURCE_INFO__ << "tab @ " << (void*)tab << ", gc = " << (void*)gc << io::endl;
		ASSERT(gc == x);
	}

	DefaultAllocator* gc;

private:
	T *tab;
	unsigned short cap, cnt;

	static unsigned long totalUse;

};


// EmbedVector methods

template <class T>
unsigned long Vector<T>::countX = 0;

template <class T>
unsigned long Vector<T>::countY = 0;

template <class T>
unsigned long Vector<T>::maxUse = 0;

template <class T>
unsigned long Vector<T>::minUse = 201240;

template <class T>
unsigned long Vector<T>::totalUse = 0;

template <class T>
inline Array<T> Vector<T>::detach(void) {
	T *dtab = tab;
	tab = 0;
	return Array<T>(dtab, cnt);
}

template <class T> int Vector<T>::capacity(void) const {
	return cap;
}
template <class T> T& Vector<T>::item(int index) {
	ASSERTP(index < cnt, "index out of bounds");
	return tab[index];
}
template <class T> const T& Vector<T>::get(int index) const {
	ASSERTP(index < cnt, "index out of bounds");
	return tab[index];
}
template <class T> void Vector<T>::set(int index, const T& value) {
	ASSERTP(index < cnt, "index out of bounds");
	tab[index] = value;
}
template <class T> bool Vector<T>::contains(const T& value) const {
	for(int i = 0; i < cnt; i++)
		if(value == tab[i])
			return true;
	return false;
}
template <class T> int Vector<T>::indexOf(const T& value, int start) const {
	for(int i = start; i < cnt; i++)
		if(value == tab[i])
			return i;
	return -1;
}
template <class T> int Vector<T>::lastIndexOf(const T& value, int start) const {
	ASSERTP(start <= cnt, "index out of bounds");
	for(int i = (start < 0 ? cnt : start) - 1; i >= 0; i--)
		if(value == tab[i])
			return i;
	return -1;
}

template <class T> void Vector<T>::add(const T& value) {
	if(cnt >= cap)
		grow(cap * 2);
	tab[cnt++] = value;
}

template <class T>
inline void Vector<T>::add(void) {
	if(cnt >= cap)
		grow(cap * 2);
	cnt++;
}

template <class T> void Vector<T>::removeAt(int index) {
	for(int i = index + 1; i < cnt; i++)
		tab[i - 1] = tab[i];
	cnt--;
}
template <class T> void Vector<T>::insert(int index, const T& value) {
	ASSERTP(index <= cnt, "index out of bounds");
	if(cnt >= cap)
		grow(cap * 2);
	for(int i = cnt; i > index; i--)
		tab[i] = tab[i - 1];
	tab[index] = value;
	cnt++;
}
template <class T> void Vector<T>::grow(int new_cap) {
	// ASSERTP(new_cap > 0 && new_cap < 65536, "new capacity out of [1, 65535]");
	ASSERTP(new_cap >= cap, "new capacity must be bigger than old one");
	if(new_cap == 0)
		new_cap = 4;
	// allocate the storage for the tab
	T *new_tab;
	if(gc == 0)
		new_tab = new T[new_cap];
	else
		new_tab = static_cast<T*>(gc->allocate(sizeof(T)*new_cap));
	// copy the values from the previous tab to the new tab
	for(int i =0; i < cnt; i++)
		new_tab[i] = tab[i];
	// clear the tab
	if((gc == 0) && (cap > 0))
		delete [] tab;
	tab = new_tab;
	cap = new_cap;
}
template <class T> void Vector<T>::setLength(int new_length) {
	int new_cap;
	ASSERTP(new_length >= 0, "new length must be >= 0");

	for (new_cap = 1; new_cap < new_length; new_cap *= 2);
	if (new_cap > cap) {
		grow(new_cap);
	}
	cnt = new_length;
}

template <class T> inline void Vector<T>::copy(const Vector& vec) {
	if(!tab || vec.cnt > cap) {
		if(tab && (gc == 0)) // only remove the tab manually if gc is not available
			delete [] tab;
		if(vec.cap == 0) // no need to copy the vector if the source size is 0
			return;

//		int newcap = 0;
//		if(vec.cnt == 0)
//			newcap = 4;
//		else
//			newcap = vec.cnt << 1;
//
//		assert(vec.cnt <= vec.cap);
		int newcap = vec.cap;
//		if(vec.cnt <= (vec.cap >> 1) && vec.cap > 1024) {
//			assert(0);
//		}
		if((vec.cnt < 4) && (vec.cap > 4)) {
			newcap = 4;
		}

		if(gc == 0)
			tab = new T[vec.cap];
		else
			//tab = static_cast<T*>(gc->allocate(sizeof(T)*((vec.cnt == 0)?4:vec.cnt << 1)));
			tab = static_cast<T*>(gc->allocate(sizeof(T)*newcap));
		//cap = vec.cap;
		cap = newcap;
	}
	cnt = vec.cnt;
	for(int i = 0; i < cnt; i++)
		tab[i] = vec.tab[i];
}

} }

#endif	// ELM_GENSTRUCT_VECTOR_H
