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
	void dif(DFABitSet *set);
	int counttruebit(void);
	
	// DFASet overload
	virtual bool equals(DFASet *set);
	virtual void add(DFASet *set);
	virtual void remove(DFASet *set);
};

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

}	// otawa

#endif	// OTAWA_UTIL_DFA_BIT_SET_H
