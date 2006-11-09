/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	util/DFABitSet.h -- DFABitSet class interface.
 */
#ifndef OTAWA_UTIL_DFA_BIT_SET_H
#define OTAWA_UTIL_DFA_BIT_SET_H

#include <assert.h>
#include <elm/io.h>
#include <elm/util/BitVector.h>
#include <otawa/util/DFA.h>

namespace otawa {

// DFABitSet class
class DFABitSet: public DFASet {
	elm::BitVector vec;
public:
	inline DFABitSet(int size, bool set = false);
	virtual ~DFABitSet(void);
	inline void add(int index);
	inline void remove(int index);
	inline bool contains(int index);
	inline void empty(void);
	inline void fill(void);
	inline void mask(DFABitSet *set);
	inline int size(void) const;
	inline void reset(void);
	void dif(DFABitSet *set);
	int counttruebit(void);
	inline elm::BitVector& vector(void) { return vec; };
	
	// DFASet overload
	inline void merge(DFABitSet *set);
	virtual bool equals(DFASet *set);
	virtual void add(DFASet *set);
	virtual void remove(DFASet *set);
};


// DFAInvBitSet class
class DFAInvBitSet {
	elm::BitVector vec;
public:
	inline DFAInvBitSet(int size);
	inline const elm::BitVector& vector(void) const;
	
	inline bool contains(int index) const;
	inline void add(int index);
	inline void remove(int index);
	
	inline bool equals(const DFAInvBitSet *set) const;
	inline bool contains(const DFAInvBitSet *set) const;
	inline bool isEmpty(void) const;
	inline bool isFull(void) const;
	
	inline void add(const DFAInvBitSet *set);
	inline void remove(const DFAInvBitSet *set);
};


// IO
elm::io::Output& operator<<(elm::io::Output& output, DFABitSet& bits);


// DFABitSet inlines
inline DFABitSet::DFABitSet(int size, bool set): vec(size, set) {
	assert(size > 0);
};

inline void DFABitSet::add(int index) {
	vec.set(index);
}

inline void DFABitSet::remove(int index) {
	vec.clear(index);
}

inline bool DFABitSet::contains(int index) {
	return vec.bit(index);
}

inline void DFABitSet::empty(void) {
	vec.clear();
}

inline void DFABitSet::fill(void) {
	vec.set();
}

inline void DFABitSet::mask(DFABitSet *set) {
	vec.applyAnd(set->vec);
}
inline void DFABitSet::dif(DFABitSet *set) {
	vec.applyReset(set->vec);	
}

inline int DFABitSet::size(void) const {
	return vec.size();
}

inline void DFABitSet::reset(void) {
	vec.clear();
}

inline void DFABitSet::merge(DFABitSet *set) {
	vec.applyOr(set->vector());
}

}	// otawa

#endif	// OTAWA_UTIL_DFA_BIT_SET_H
