/*
 *	DynamicBranching Features
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
#ifndef OTAWA_DYNAMIC_BRANCHING_FEATURES_H
#define OTAWA_DYNAMIC_BRANCHING_FEATURES_H

#include <otawa/otawa.h>
#include <elm/alloc/StackAllocator.h>

namespace otawa { namespace dynbranch {

#ifndef OTAWA_DYNBRANCH_SET_H
#define OTAWA_DYNBRANCH_SET_H
template <typename T>
class Set : public genstruct::Vector<T> {
public:

	Set<T>(void) : genstruct::Vector<T>() { }

	void insert(const T& val) {
		if(!genstruct::Vector<T>::contains(val))
			genstruct::Vector<T>::push(val);
	}
};
#endif

#ifndef OTAWA_DYNBRANCH_POTENTIAL_VALUE_H
#define OTAWA_DYNBRANCH_POTENTIAL_VALUE_H
class PotentialValueMem;

class PotentialValue: public Set<elm::t::uint32> {
	friend Output& operator<<(Output& o, PotentialValue const& pv);
public:
	PotentialValue(void);
	PotentialValue(const PotentialValue& cpv);
	PotentialValue& operator=(const PotentialValue& obj);
	~PotentialValue(void);
	// for FastState:
	typedef PotentialValue t;
	static PotentialValue bot;
	static PotentialValue top;
	static PotentialValue DEFAULT;

	inline void dump(Output& o, PotentialValue &pv) {
		o << "{ ";
		for(PotentialValue::Iterator i(pv); i; i++)
			o << "0x" << hex(*i) << " ";
		o << "}";
	}

	inline bool equals(PotentialValue &a, PotentialValue &b) { return a == b; }
	static unsigned int MAGIC;
	unsigned int magic;
#ifdef SAFE_MEM_ACCESS
	static SLList<PotentialValueMem*> potentialValueCollector;
	typedef SLList<PotentialValueMem*> potential_value_list_t;
	PotentialValueMem* pvm;
#else
	static SLList<PotentialValue*> potentialValueCollector;
	typedef SLList<PotentialValue*> potential_value_list_t;
#endif
};

PotentialValue operator&(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator+(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator|(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator^(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator~(const PotentialValue& a);
PotentialValue operator-(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator||(const PotentialValue& a, const PotentialValue& b);
bool operator==(const PotentialValue& a, const PotentialValue& b);
PotentialValue merge(const PotentialValue& a, const PotentialValue& b);
PotentialValue logicalShiftRight(const PotentialValue& a, const PotentialValue& b);

#ifdef SAFE_MEM_ACCESS
class PotentialValueMem {
public:
	PotentialValue* pv;
	bool status;
};
#endif
#endif

extern p::feature FEATURE;
extern Identifier<bool> TIME ;

// if any target is found in the dynamic branching analysis
// if the values of DYNBRANCH_TARGET_COUNT and DYNBRANCH_TARGET_COUNT_PREV differ, it means that there is a new target found, NEW_BRANCH_TARGET_FOUND will be set to true
extern Identifier<bool> NEW_BRANCH_TARGET_FOUND;
    
// the current branching target counts for a given instruction
extern Identifier<int> DYNBRANCH_TARGET_COUNT;

// the previous branching target counts for a given instruction
extern Identifier<int> DYNBRANCH_TARGET_COUNT_PREV;

extern Identifier<elm::StackAllocator*> DYNBRANCH_STACK_ALLOCATOR;

extern Identifier<PotentialValue::potential_value_list_t* > DYNBRANCH_POTENTIAL_VALUE_LIST;

} } // otawa::dynbranch

#endif	// OTAWA_DYNAMIC_BRANCHING_ANALYSIS_H

