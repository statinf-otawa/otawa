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

#include <elm/util/BitVector.h>

namespace otawa { namespace dfa {

// BitSet class
class BitSet {
	elm::BitVector vec;
	inline BitSet(const elm::BitVector& ivec);
public:
	inline BitSet(void) { }
	inline BitSet(int size);
	inline BitSet(const BitSet& set);
	
	inline bool isEmpty(void) const;
	inline bool isFull(void) const;
	inline void fill(void);
	inline void empty(void);
	
	inline bool contains(int index) const;
	inline void add(int index);
	inline void remove(int index);

	inline bool equals(const BitSet& set) const;
	inline bool includes(const BitSet& set) const;
	inline bool includesStrictly(const BitSet& set) const;
	inline void complement(void);
	inline int size(void) const;
	inline void add(const BitSet& set);
	inline void remove(const BitSet& set);
	inline void mask(const BitSet& set);
	inline int count(void) const;

	inline BitSet doComp(void) const;	
	inline BitSet doUnion(const BitSet& set) const;
	inline BitSet doInter(const BitSet& set) const;
	inline BitSet doDiff(const BitSet& set) const;
	
	inline BitSet& operator=(const BitSet& set);

	inline BitSet& operator+=(int index);
	inline BitSet& operator+=(const BitSet& set);
	inline BitSet& operator-=(int index);
	inline BitSet& operator-=(const BitSet& set);
	inline BitSet operator+(const BitSet& set);
	inline BitSet operator-(const BitSet& set);
	
	inline BitSet& operator|=(const BitSet& set);	
	inline BitSet& operator&=(const BitSet& set);
	inline BitSet operator|(const BitSet& set);
	inline BitSet operator&(const BitSet& set);
	
	inline bool operator==(const BitSet& set) const;
	inline bool operator!=(const BitSet& set) const;
	inline bool operator>=(const BitSet& set) const;
	inline bool operator<=(const BitSet& set) const;
	inline bool operator>(const BitSet& set) const;
	inline bool operator<(const BitSet& set) const;
	
	// Iterator class
	class Iterator: public elm::BitVector::OneIterator {
	public:
		inline Iterator(const BitSet& set);
	};
};

inline elm::io::Output& operator<<(elm::io::Output& output, const BitSet& bits) {
        output << "{";
        bool first = true;
        for(int i = 0; i < bits.size(); i++)
                if(bits.contains(i)) {
                        if(first)
                                first = false;
                        else
                                output << ", ";
                        output << i;
                }
        output << "}";
        return output;

}

// BitSet inlines
inline BitSet::BitSet(const elm::BitVector& ivec): vec(ivec) {
}

inline BitSet::BitSet(int size): vec(size) {
}

inline BitSet::BitSet(const BitSet& set): vec(set.vec) {
}
	
inline bool BitSet::isEmpty(void) const {
	return vec.isEmpty();
}

inline bool BitSet::isFull(void) const {
	return vec.countBits() == vec.size();
}

inline int BitSet::size(void) const {
	return vec.size();
}


inline int BitSet::count(void) const {
	return vec.countBits();
}
inline void BitSet::fill(void) {
	vec.set();
}

inline void BitSet::empty(void) {
	vec.clear();
}
	
inline bool BitSet::contains(int index) const {
	return vec.bit(index);
}

inline void BitSet::add(int index) {
	vec.set(index);
}

inline void BitSet::remove(int index) {
	vec.clear(index);
}

inline bool BitSet::equals(const BitSet& set) const {
	return vec.equals(set.vec);
}

inline bool BitSet::includes(const BitSet& set) const {
	return vec.includes(set.vec);
}

inline bool BitSet::includesStrictly(const BitSet& set) const {
	return vec.includesStrictly(set.vec);
}

inline void BitSet::complement(void) {
	vec.applyNot();
}

inline void BitSet::add(const BitSet& set) {
	vec.applyOr(set.vec);
}

inline void BitSet::remove(const BitSet& set) {
	vec.applyReset(set.vec);
}

inline void BitSet::mask(const BitSet& set) {
	vec.applyAnd(set.vec);
}

inline BitSet BitSet::doComp(void) const {
	return BitSet(vec.makeNot());
}

inline BitSet BitSet::doUnion(const BitSet& set) const {
	return BitSet(vec.makeOr(set.vec));
}

inline BitSet BitSet::doInter(const BitSet& set) const {
	return BitSet(vec.makeAnd(set.vec));
}

inline BitSet BitSet::doDiff(const BitSet& set) const {
	return BitSet(vec.makeReset(set.vec));
}

inline BitSet& BitSet::operator=(const BitSet& set) {
	vec = set.vec;
	return *this;
}

inline BitSet& BitSet::operator+=(int index) {
	add(index);
	return *this;
}

inline BitSet& BitSet::operator+=(const BitSet& set) {
	add(set);
	return *this;
}

inline BitSet& BitSet::operator-=(int index) {
	remove(index);
	return *this;
}

inline BitSet& BitSet::operator-=(const BitSet& set) {
	remove(set);
	return *this;
}

inline BitSet BitSet::operator+(const BitSet& set) {
	return doUnion(set);
}

inline BitSet BitSet::operator-(const BitSet& set) {
	return doDiff(set);
}
	
inline BitSet& BitSet::operator|=(const BitSet& set) {
	add(set);
	return *this;
}
	
inline BitSet& BitSet::operator&=(const BitSet& set) {
	mask(set);
	return *this;
}

inline BitSet BitSet::operator|(const BitSet& set) {
	return doUnion(set);
}

inline BitSet BitSet::operator&(const BitSet& set) {
	return doInter(set);
}
	
inline bool BitSet::operator==(const BitSet& set) const {
	return equals(set);
}

inline bool BitSet::operator!=(const BitSet& set) const {
	return !equals(set);
}

inline bool BitSet::operator>=(const BitSet& set) const {
	return includes(set);
}

inline bool BitSet::operator<=(const BitSet& set) const {
	return set.includes(*this);
}

inline bool BitSet::operator>(const BitSet& set) const {
	return includesStrictly(set);
}

inline bool BitSet::operator<(const BitSet& set) const {
	return set.includesStrictly(*this);
}


// BitVector::Iterator inlines
inline BitSet::Iterator::Iterator(const BitSet& set)
: elm::BitVector::OneIterator(set.vec) {
}

} } // otawa::dfa

#endif // OTAWA_DFA_BITSET_H
