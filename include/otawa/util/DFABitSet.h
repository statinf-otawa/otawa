/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	util/DFABitSet.h -- DFABitSet class interface.
 */
#ifndef OTAWA_UTIL_DFA_BIT_SET_H
#define OTAWA_UTIL_DFA_BIT_SET_H

#include <elm/util/BitVector.h>
#include <otawa/util/DFA.h>

namespace otawa {

// DFABitSet class
class DFABitSet: public DFASet {
	elm::BitVector vec;
public:
	inline DFABitSet(int size, bool set = false);
	~DFABitSet(void);
	//virtual ~DFABitSet(void);
	inline void add(int index);
	inline void remove(int index);
	inline bool contains(int index);
	
	// DFASet overload
	virtual void reset(void);
	virtual bool equals(DFASet *set);
	virtual void add(DFASet *set);
	virtual void remove(DFASet *set);
};

// Inlines
inline DFABitSet::DFABitSet(int size, bool set): vec(size) {
	if(set)
		vec.applyNot();
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

}	// otawa

#endif	// OTAWA_UTIL_DFA_BIT_SET_H
