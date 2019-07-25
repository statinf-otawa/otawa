/*
 *	BitSet class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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
#ifndef OTAWA_DFA_BITSET_H
#define OTAWA_DFA_BITSET_H

#define OTAWA_BITSET_SIZE
//#define OTAWA_BITSET_BOTH

#if defined(OTAWA_BITSET_WAH) || defined(OTAWA_BITSET_BOTH)
#	include <elm/util/WAHVector.h>
#endif
#if defined(OTAWA_BITSET_BOTH) || !defined(OTAWA_BITSET_WAH)
#	include <elm/util/BitVector.h>
#endif
#ifdef OTAWA_BITSET_BOTH
#	include <elm/util/WAHVector.h>
#	include <elm/util/BitVector.h>
	class Both {
	public:
		Both(void) { }
		Both(int s, bool v = false): bvec(s, v), wvec(s, v) { }
		inline int __size(void) const { return bvec.__size() + wvec.__size(); }
		inline bool isEmpty(void) const { ASSERT(bvec.isEmpty() && wvec.isEmpty()); return bvec.isEmpty(); }
		inline int countBits(void) const { ASSERT(bvec.countBits() == wvec.countBits()); return bvec.countBits(); };
		inline int size(void) const { ASSERT(bvec.size() == wvec.size()); return bvec.size(); }
		inline void set(void) { bvec.set(); wvec.set(); check(); }
		inline void clear(void) { bvec.clear(); wvec.clear(); check(); }
		inline bool bit(int i) const { ASSERT(bvec.bit(i) == wvec.bit(i)); return bvec.bit(i); }
		inline void set(int i) { bvec.set(i); wvec.set(i); check(); }
		inline void clear(int i) { bvec.clear(i); wvec.clear(i); check(); }
		inline bool equals(const Both& b) const { ASSERT(bvec.equals(b.bvec) == wvec.equals(b.wvec)); return bvec.equals(b.bvec); }
		inline bool includes(const Both& b) const { ASSERT(bvec.includes(b.bvec) == wvec.includes(b.wvec)); return bvec.includes(b.bvec); }
		inline bool includesStrictly(const Both& b) const { ASSERT(bvec.includesStrictly(b.bvec) == wvec.includesStrictly(b.wvec)); return bvec.includesStrictly(b.bvec); }
		inline void applyNot(void) { bvec.applyNot(); wvec.applyNot(); check(); }
		inline void applyOr(const Both& b) { bvec.applyOr(b.bvec); wvec.applyOr(b.wvec); check(); }
		inline void applyAnd(const Both& b) { bvec.applyAnd(b.bvec); wvec.applyAnd(b.wvec); check(); }
		inline void applyReset(const Both& b) { bvec.applyReset(b.bvec); wvec.applyReset(b.wvec); check(); }
		inline Both makeNot(void) const { Both b; b.bvec = bvec.makeNot(); b.wvec = wvec.makeNot(); b.check(); return b; }
		inline Both makeOr(const Both& b) const { Both r; r.bvec = bvec.makeOr(b.bvec); r.wvec = wvec.makeOr(b.wvec); r.check(); return r; }
		inline Both makeAnd(const Both& b) const { Both r; r.bvec = bvec.makeAnd(b.bvec); r.wvec = wvec.makeAnd(b.wvec); r.check(); return r; }
		inline Both makeReset(const Both& b) const { Both r; r.bvec = bvec.makeReset(b.bvec); r.wvec = wvec.makeReset(b.wvec); r.check(); return r; }

		class OneIterator: public elm::BitVector::OneIterator {
		public:
			OneIterator(const Both& b): elm::BitVector::OneIterator(b.bvec) { }
		};

	private:
		elm::BitVector bvec;
		elm::WAHVector wvec;

		void check(void) {
			ASSERT(bvec.size() == wvec.size());
			for(int i = 0; i < bvec.size(); i++)
				ASSERT(bvec[i] == wvec[i]);
		}
	};
#endif

namespace otawa { namespace dfa {

// BitSet class
class BitSet {
#	ifdef OTAWA_BITSET_WAH
		typedef elm::WAHVector vec_t;
#	elif defined(OTAWA_BITSET_BOTH)
		typedef Both vec_t;
#	else
		typedef elm::BitVector vec_t;
#	endif
	vec_t vec;
public:
#	ifdef OTAWA_BITSET_SIZE
		static int __used_size, __max_size, __total_size;
#		define OTAWA_BITSET_ALLOC	{ __total_size += vec.__size(); __used_size += vec.__size(); if(__used_size >= __max_size) __max_size = __used_size; }
#		define OTAWA_BITSET_FREE	{ __used_size -= vec.__size(); }
#	else
#		define OTAWA_BITSET_ALLOC
#		define OTAWA_BITSET_FREE
#	endif

	inline BitSet(void) { OTAWA_BITSET_ALLOC }
	inline BitSet(int size): vec(size) { OTAWA_BITSET_ALLOC }
	inline BitSet(const BitSet& set): vec(set.vec) { OTAWA_BITSET_ALLOC }
	inline ~BitSet(void) { OTAWA_BITSET_FREE }

	inline bool isEmpty(void) const { return vec.isEmpty(); }
	inline bool isFull(void) const { return vec.countBits() == vec.size(); }
	inline void fill(void) { vec.set(); }
	inline void empty(void) { vec.clear(); }

	inline bool contains(int index) const { return vec.bit(index); }
	inline void add(int index) { OTAWA_BITSET_FREE vec.set(index); OTAWA_BITSET_ALLOC }
	inline void remove(int index) { OTAWA_BITSET_FREE vec.clear(index); OTAWA_BITSET_ALLOC }

	inline bool equals(const BitSet& set) const { return vec.equals(set.vec); }
	inline bool includes(const BitSet& set) const { return vec.includes(set.vec); }
	inline bool includesStrictly(const BitSet& set) const { return vec.includesStrictly(set.vec); }
	inline void complement(void) { vec.applyNot(); }
	inline int size(void) const { return vec.size(); }
	inline void add(const BitSet& set) { OTAWA_BITSET_FREE vec.applyOr(set.vec); OTAWA_BITSET_ALLOC }
	inline void remove(const BitSet& set) { OTAWA_BITSET_FREE vec.applyReset(set.vec); OTAWA_BITSET_ALLOC }
	inline void mask(const BitSet& set) { OTAWA_BITSET_FREE vec.applyAnd(set.vec); OTAWA_BITSET_ALLOC }
	inline int count(void) const { return vec.countBits(); }
	inline bool meets(const BitSet& set) { return vec.meets(set.vec); }

	inline BitSet doComp(void) const { return BitSet(vec.makeNot()); }
	inline BitSet doUnion(const BitSet& set) const { return BitSet(vec.makeOr(set.vec)); }
	inline BitSet doInter(const BitSet& set) const { return BitSet(vec.makeAnd(set.vec)); }
	inline BitSet doDiff(const BitSet& set) const { return BitSet(vec.makeReset(set.vec)); }

	inline BitSet& operator=(const BitSet& set) { OTAWA_BITSET_FREE vec = set.vec; OTAWA_BITSET_ALLOC return *this; }

	inline BitSet& operator+=(int index) { add(index); return *this; }
	inline BitSet& operator+=(const BitSet& set) { add(set); return *this; }
	inline BitSet& operator-=(int index) { remove(index); return *this; }
	inline BitSet& operator-=(const BitSet& set) { remove(set); return *this; }
	inline BitSet operator+(const BitSet& set) { return doUnion(set); }
	inline BitSet operator-(const BitSet& set) { return doDiff(set); }

	inline BitSet& operator|=(const BitSet& set) { add(set); return *this; }
	inline BitSet& operator&=(const BitSet& set) { mask(set); return *this; }
	inline BitSet operator|(const BitSet& set) { return doUnion(set); }
	inline BitSet operator&(const BitSet& set) { return doInter(set); }

	inline bool operator==(const BitSet& set) const { return equals(set); }
	inline bool operator!=(const BitSet& set) const { return !equals(set); }
	inline bool operator>=(const BitSet& set) const { return includes(set); }
	inline bool operator<=(const BitSet& set) const { return set.includes(*this); }
	inline bool operator>(const BitSet& set) const { return includesStrictly(set); }
	inline bool operator<(const BitSet& set) const { return set.includesStrictly(*this); }

	// Iterator class
	class Iterator: public vec_t::OneIterator {
	public:
		inline Iterator(const BitSet& set): vec_t::OneIterator(set.vec) { }
	};

private:
	inline BitSet(const vec_t& ivec): vec(ivec) { }
};

elm::io::Output& operator<<(elm::io::Output& out, const BitSet& bits);
inline elm::io::Output& operator<<(elm::io::Output& out, const BitSet *bits) { out << *bits; return out; }

} } // otawa::dfa

#endif // OTAWA_DFA_BITSET_H
