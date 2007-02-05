/*
 *	$Id$
 *	Copyright (c) 2006-07, IRIT UPS.
 *
 *	util::BitSet class interface
 */
#ifndef OTAWA_UTIL_BITSET_H
#define OTAWA_UTIL_BITSET_H

#include <otawa/dfa/BitSet.h>

#warning "Deprecared header and classes. Use otawa/dfa/BitSet.h instead."

namespace otawa {

// BitSet class
class BitSet: public dfa::BitSet {
public:
	inline BitSet(int size): dfa::BitSet(size) { }
	inline BitSet(const BitSet& set): dfa::BitSet(set) { }
};

} // otawa

#endif // OTAWA_UTIL_BITSET_H
