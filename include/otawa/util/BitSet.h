/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	util/BitSet.h -- BitSet class interface.
 */
#ifndef OTAWA_UTIL_BITSET_H
#define OTAWA_UTIL_BITSET_H

#include <elm/util/BitVector.h>

namespace otawa {

// BitSet class
class BitSet {
	elm::BitVector vec;
	inline BitSet(const elm::BitVector& ivec);
public:
	inline BitSet(int size);
	inline BitSet(const BitSet& set);
	
	bool isEmpty(void) const;
	bool isFull(void) const;
	void fill(void);
	void empty(void);
	
	bool contains(int index) const;
	void add(int index);
	void remove(int index);

	bool equals(const BitSet& set) const;
	bool includes(const BitSet& set) const;
	bool includesStrictly(const BitSet& set) const;
	void complement(void);
	void add(const BitSet& set);
	void remove(const BitSet& set);
	void mask(const BitSet& set);

	BitSet doComp(void) const;	
	BitSet doUnion(const BitSet& set) const;
	BitSet doInter(const BitSet& set) const;
	BitSet doDiff(const BitSet& set) const;
	
	BitSet& operator=(const BitSet& set);

	BitSet& operator+=(int index);
	BitSet& operator+=(const BitSet& set);
	BitSet& operator-=(int index);
	BitSet& operator-=(const BitSet& set);
	BitSet operator+(const BitSet& set);
	BitSet operator-(const BitSet& set);
	
	BitSet& operator|=(const BitSet& set);	
	BitSet& operator&=(const BitSet& set);
	BitSet operator|(const BitSet& set);
	BitSet operator&(const BitSet& set);
	
	bool operator==(const BitSet& set) const;
	bool operator!=(const BitSet& set) const;
	bool operator>=(const BitSet& set) const;
	bool operator<=(const BitSet& set) const;
	bool operator>(const BitSet& set) const;
	bool operator<(const BitSet& set) const;
	
	// Iterator class
	class Iterator: public elm::BitVector::OneIterator {
	public:
		inline Iterator(const BitSet& set);
	};
};

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

bool BitSet::isFull(void) const {
	return vec.countBits() == vec.size();
}

void BitSet::fill(void) {
	vec.set();
}

void BitSet::empty(void) {
	vec.clear();
}
	
bool BitSet::contains(int index) const {
	return vec.bit(index);
}

void BitSet::add(int index) {
	vec.set(index);
}

void BitSet::remove(int index) {
	vec.clear(index);
}

bool BitSet::equals(const BitSet& set) const {
	return vec.equals(set.vec);
}

bool BitSet::includes(const BitSet& set) const {
	return vec.includes(set.vec);
}

bool BitSet::includesStrictly(const BitSet& set) const {
	return vec.includesStrictly(set.vec);
}

void BitSet::complement(void) {
	vec.applyNot();
}

void BitSet::add(const BitSet& set) {
	vec.applyOr(set.vec);
}

void BitSet::remove(const BitSet& set) {
	vec.applyReset(set.vec);
}

void BitSet::mask(const BitSet& set) {
	vec.applyAnd(set.vec);
}

BitSet BitSet::doComp(void) const {
	return BitSet(vec.makeNot());
}

BitSet BitSet::doUnion(const BitSet& set) const {
	return BitSet(vec.makeOr(set.vec));
}

BitSet BitSet::doInter(const BitSet& set) const {
	return BitSet(vec.makeAnd(set.vec));
}

BitSet BitSet::doDiff(const BitSet& set) const {
	return BitSet(vec.makeReset(set.vec));
}

BitSet& BitSet::operator=(const BitSet& set) {
	vec = set.vec;
	return *this;
}

BitSet& BitSet::operator+=(int index) {
	add(index);
	return *this;
}

BitSet& BitSet::operator+=(const BitSet& set) {
	add(set);
	return *this;
}

BitSet& BitSet::operator-=(int index) {
	remove(index);
	return *this;
}

BitSet& BitSet::operator-=(const BitSet& set) {
	remove(set);
	return *this;
}

BitSet BitSet::operator+(const BitSet& set) {
	return doUnion(set);
}

BitSet BitSet::operator-(const BitSet& set) {
	return doDiff(set);
}
	
BitSet& BitSet::operator|=(const BitSet& set) {
	add(set);
	return *this;
}
	
BitSet& BitSet::operator&=(const BitSet& set) {
	mask(set);
	return *this;
}

BitSet BitSet::operator|(const BitSet& set) {
	return doUnion(set);
}

BitSet BitSet::operator&(const BitSet& set) {
	return doInter(set);
}
	
bool BitSet::operator==(const BitSet& set) const {
	return equals(set);
}

bool BitSet::operator!=(const BitSet& set) const {
	return !equals(set);
}

bool BitSet::operator>=(const BitSet& set) const {
	return includes(set);
}

bool BitSet::operator<=(const BitSet& set) const {
	return set.includes(*this);
}

bool BitSet::operator>(const BitSet& set) const {
	return includesStrictly(set);
}

bool BitSet::operator<(const BitSet& set) const {
	return set.includesStrictly(*this);
}


// BitVector::Iterator inlines
inline BitSet::Iterator::Iterator(const BitSet& set)
: elm::BitVector::OneIterator(set.vec) {
}

} // otawa

#endif // OTAWA_UTIL_BITSET_H
