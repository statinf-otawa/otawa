/*
 *	PotentialValue class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_DYNBRANCH_POTENTIAL_VALUE_H
#define OTAWA_DYNBRANCH_POTENTIAL_VALUE_H

#include "Set.h"
#include <elm/io/OutStream.h>
#include <elm/data/List.h>
#include <otawa/prop/Identifier.h>

using namespace elm;
using namespace elm::io;

namespace otawa { namespace dynbranch {


class MyGC;


#ifdef SAFE_MEM_ACCESS
class PotentialValueMem;
typedef SLList<PotentialValueMem*> potential_value_list_t;
#else
class PotentialValue;
typedef List<PotentialValue*> potential_value_list_t;
#endif

typedef t::uint32 potential_value_type ;

//class PotentialValue: public Set<potential_value_type> {
class PotentialValue: public Vector<potential_value_type> {
	friend Output& operator<<(Output& o, PotentialValue const& pv);
	friend bool operator==(const PotentialValue& a, const PotentialValue& b);
public:
//    inline void* operator new(std::size_t sz) { elm::cout << "donc1 " << sz << io::endl; return ::operator new(sz);}
//    inline void* operator new[](std::size_t sz) { std::cout << "donc2 " << sz << '\n'; return ::operator new(sz); }
//	template<class T> inline void *operator new(size_t size, T& alloc) { elm::cout << "donc3 " << io::endl; return alloc.template allocate(size); }
//	template<class T> inline void *operator new[](size_t size, T& alloc) { elm::cout << "donc4 " << size << io::endl; return alloc.template allocate(size); }
	PotentialValue(bool top = false);
	PotentialValue(const PotentialValue& cpv);
	PotentialValue& operator=(const PotentialValue& obj);
	~PotentialValue(void);
	// for FastState:
	typedef PotentialValue t; // use as the type of the Domain
	static PotentialValue bot; // ⊥
	static PotentialValue top; // ⊤
	static PotentialValue DEFAULT;
	static const PotentialValue* tempPVAlloc;
	inline void dump(Output& o, PotentialValue &pv) { o << pv; }

	inline bool equals(const PotentialValue &a, const PotentialValue &b) { return a == b; }
	bool collect(const MyGC*  gc, bool show = false) const;

	inline void checkState() { Vector<potential_value_type>::checkState(pvgc); }
	inline void checkState() const { Vector<potential_value_type>::checkState(pvgc); }


	inline void insert(unsigned int val) {
		if(!contains(val))
			push(val);
	}


	inline void sum(const PotentialValue& a, const PotentialValue& b) {
		if(a.count() == 0 || b.count() == 0) {
			clear();
			bTop = false;
			return;
		}

		if(a.count()*b.count() >= 10000) {
			elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
			clear();
			bTop = false;
			return;
		}
		for(PotentialValue::Iterator ita(a); ita(); ita++)
			for(PotentialValue::Iterator itb(b); itb(); itb++)
				insert(*ita + *itb);
	}


	static unsigned int MAGIC;
//	unsigned int magic;
//	static potential_value_list_t potentialValueCollector;
	bool bTop;
//	static int countX;
//	static int countY;
	static MyGC* pvgc;
#ifdef SAFE_MEM_ACCESS
	PotentialValueMem* pvm;
#endif
};

// can only use at the right hand side
PotentialValue operator&(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator+(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator|(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator*(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator^(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator~(const PotentialValue& a);
PotentialValue operator-(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b);
PotentialValue operator||(const PotentialValue& a, const PotentialValue& b);
bool operator==(const PotentialValue& a, const PotentialValue& b);
PotentialValue merge(const PotentialValue& a, const PotentialValue& b);
PotentialValue logicalShiftRight(const PotentialValue& a, const PotentialValue& b);
PotentialValue MULH(const PotentialValue& a, const PotentialValue& b);
PotentialValue DIV(const PotentialValue& a, const PotentialValue& b);
PotentialValue DIVU(const PotentialValue& a, const PotentialValue& b);

#ifdef SAFE_MEM_ACCESS
class PotentialValueMem {
public:
	PotentialValue* pv;
	bool status;
};
#endif

//extern Identifier<potential_value_list_t* > DYNBRANCH_POTENTIAL_VALUE_LIST;
}}
#endif	// OTAWA_DYNBRANCH_POTENTIAL_VALUE_H

