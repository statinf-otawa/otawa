/*
 *	PotentialValue class implementation
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

#include <otawa/dynbranch/features.h>
#include <include/otawa/proc/Monitor.h>
#include "PotentialValue.h"
#include "State.h"
#include "GlobalAnalysisProblem.h"
#include "GC.h"

namespace otawa { namespace dynbranch {

//Identifier<potential_value_list_t*> DYNBRANCH_POTENTIAL_VALUE_LIST("");

// these three potential values will stay in the heap outside the garbage collection
PotentialValue PotentialValue::bot(false);
PotentialValue PotentialValue::top(true);
PotentialValue PotentialValue::DEFAULT(false);
const PotentialValue* PotentialValue::tempPVAlloc = 0;

unsigned int PotentialValue::MAGIC = 0;
MyGC* PotentialValue::pvgc = 0;

#ifdef SAFE_MEM_ACCESS
SLList<PotentialValueMem*> PotentialValue::potentialValueCollector;
#else
//SLList<PotentialValue*> PotentialValue::potentialValueCollector;
#endif

//int PotentialValue::countX = 0;
//int PotentialValue::countY = 0;

//#define SHOWSHOW
//PotentialValue::PotentialValue(bool _top): bTop(_top), Set<elm::t::uint32>(pvgc) {
PotentialValue::PotentialValue(bool _top): Vector<elm::t::uint32>(pvgc, 0), bTop(_top) {
	//magic = MAGIC;
/*
#ifdef SHOWSHOW
	countX++;
	if(countX%1000000 == 0) {
		elm::cout << "PotentialValue created count: " << countX << ", size = " << sizeof(*this) << io::endl;
		struct mallinfo info;

		info = mallinfo();

		printf ("total allocated space:  %ul bytes\n", info.uordblks);
		printf ("total free space:       %ul bytes\n", info.fordblks);
	}
#endif
*/
}

//PotentialValue::PotentialValue(const PotentialValue & cpv) : Set<elm::t::uint32>(cpv, pvgc) {
PotentialValue::PotentialValue(const PotentialValue & cpv) :  Vector<elm::t::uint32>(pvgc, cpv) {
	// note: when a potentialvalue is created through the copy constructor, the tab is not initialized.
	// we need to call the constructor of Set and then Vector to initialize the tab
	// reminder: PotentialValue does not allocate memory for the tab hence the size of PV only includes the pointer to the tab
	bTop = cpv.bTop;
}

//PotentialValue::~PotentialValue() { ~Set<elm::t::uint32>(); memset(this, 0, sizeof(PotentialValue)); }

PotentialValue::~PotentialValue() {
//	magic = 0;
//#ifdef SHOWSHOW
//	countY++;
//	if(countY%1000000 == 0) {
//		elm::cout << "PotentialValue destroyed count: " << countY << ", size = " << sizeof(*this) << io::endl;
//
//	}
//#endif
#ifdef SAFE_MEM_ACCESS
	if(pvm)
		pvm->status = false;
#endif
//	~Set<elm::t::uint32>();
}

PotentialValue& PotentialValue::operator=(const PotentialValue& a) {
	// first copy the bTop
	bTop = a.bTop;
	// to save more time, call the operator directly
	PotentialValue::tempPVAlloc = &a;
	Vector<elm::t::uint32>::operator=(a);
	PotentialValue::tempPVAlloc = 0;
	return *this;

//	elm::cout << "pv = is called" << io::endl;
//	// need this for FastState because the memory are initialized as chars but not as the objects
//	// (!)LESSON LEARNT: when calling =, the target (this) may not be initialized yet,
//	// which means the capacity is 0, this is a big nono, hence we we grow the capacity with the same size of a
//
//	// ok someone has to be dirty
//	// FIXME: possible memory leakage! because we don't know 'this' is just created by
//	// fast state (which has mumbo jumbo values which play bad tricks) or already existed.
//	// clear the already existed potential value will cause memory leakage, say, it will make
//	// the 'tab' and 'cap' (inh. from Set and then Vector) 0, and make the operator= from Vector (which then calls its
//	// copy function) see there is no 'tab' and creates one, and make a new 'tab' without clearing the old one.
//	// But if we do not set it to 0, the 'tab' could have some random value, such as 0x7, and cap has smaller value
//	// then a.cap. the 'tab' will be used....then it leads to disaster
//	// and operator= from Vector will just use the tab
//
//	// first, we need to see if the 'tab' of this really contains something ?
//	// lets use a magic number stored in the static variable of the class MAGIC
//	// if the PotentialValue is created via ordinary constructor, then magic number should be MAGIC
//	// if not, it will be random value (could be MAGIC too, but lets bet?), we change the MAGIC for each new allocator because
//	// it is possible that allocator gets the same chunk of memory previously and the value persists.
//	// if it is not MAGIC, we know it is created by faststate, then we need to initialize the content of
//	// the memory. and assigned the magic number to MAGIC (just like a 'PASS' stamp!)
//	// we then use the operator= from Vector to create the tab
//
//	if(magic != MAGIC) { // valgrind will report error on reading magic as depends on uninitialized value(s), which is true :)
//		memset(this, 0, sizeof(PotentialValue));
//		magic = MAGIC;
//		Vector<elm::t::uint32>::operator=(a);
//#ifdef SAFE_MEM_ACCESS
//		pvm = new PotentialValueMem();
//		pvm->pv = this;
//		pvm->status = true;
//		potentialValueCollector.add(pvm);
//#else
//		potentialValueCollector.add(this);
//#endif
//	}
//	else{
//		clear();
//		for(PotentialValue::Iterator ita(a); ita; ita++)
//			insert(*ita);
//	}
//	bTop = a.bTop;
//	return *this;
}

PotentialValue operator&(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita & *itb);
	return res;
}

PotentialValue operator|(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita | *itb);
	return res;
}

PotentialValue operator^(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita ^ *itb);
	return res;
}

PotentialValue operator~(const PotentialValue& a) {
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		res.insert(~(*ita));
	return res;
}

PotentialValue operator+(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita + *itb);
	return res;
}

PotentialValue operator-(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita - *itb);
	return res;
}

PotentialValue operator*(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert((*ita)*(*itb));
	return res;
}

PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++) {
		for(PotentialValue::Iterator itb(b); itb(); itb++) {
			res.insert(*ita >> *itb);
		}
	}
	return res;
}

PotentialValue logicalShiftRight(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++) {
		for(PotentialValue::Iterator itb(b); itb(); itb++) {
			res.insert((unsigned int)*ita >> *itb);
		}
	}
	return res;
}

PotentialValue MULH(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++) {
			t::int64 temp = (*ita)*(*itb);
			t::uint32 temp2 = temp >> 32;
			res.insert(temp2);
		}
	return res;
}

PotentialValue DIV(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++) {
			if(*itb == 0)
				return PotentialValue::top;

			res.insert((*ita)/(*itb));
		}
	return res;
}

PotentialValue DIVU(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++) {
			if(*itb == 0)
				return PotentialValue::top;

			t::int64 temp = (*ita)/(*itb);
			t::uint32 temp2 = temp >> 32;
			res.insert(temp2);
		}
	return res;
}

PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita << *itb);
	return res;
}

PotentialValue operator||(const PotentialValue& a, const PotentialValue& b) {
	if(a.count() == 0 || b.count() == 0)
		return PotentialValue::bot;
	if(a.count()*b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " X " << b.count() << " = " << (a.count()*b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		for(PotentialValue::Iterator itb(b); itb(); itb++)
			res.insert(*ita || *itb);
	return res;
}

PotentialValue merge(const PotentialValue& a, const PotentialValue& b) { // result a set which contains both a and b
	if(a.count() == 0 && b.count() == 0)
		return PotentialValue::bot;
	if(a.count()+b.count() >= POTENTIAL_VALUE_WARNING_SIZE) {
		elm::cerr << "WARNING: large set of potential value with size = " << a.count() << " + " << b.count() << " = " << (a.count()+b.count()) << " @ " << __FILE__ << ":" << __LINE__ << io::endl;
		return PotentialValue::bot;
	}
	PotentialValue res;
	PotentialValue::tempPVAlloc = &res;
	for(PotentialValue::Iterator ita(a); ita(); ita++)
		res.insert(*ita);
	for(PotentialValue::Iterator itb(b); itb(); itb++) {
		res.insert(*itb);
	}
	return res;
}

bool PotentialValue::collect(const MyGC*  gc, bool show) const {
	return Vector<unsigned int>::collect();
}

bool operator==(const PotentialValue& a, const PotentialValue& b) {
	// in potential value, top and bot are no difference.... (?), hence to speed up, if they both contains 0 element, return true
	if(a.length() == 0 && b.length() == 0)
		return true;

	/*
	if(a.bTop != b.bTop) {
		return false;
	}
	*/

	if(a.length() == b.length()) {
		for(PotentialValue::Iterator isa(a); isa(); isa++)
			if(!b.contains(*isa))
				return false;
		return true;
	}
	else
		return false;

	/*
	for(PotentialValue::Iterator isa(a); isa; isa++)
		if(!b.contains(*isa))
			return false;
	for(PotentialValue::Iterator isb(b); isb; isb++)
		if(!a.contains(*isb))
			return false;
	return true;
	*/
}

Output& operator<<(Output& o, PotentialValue const& pv) {
	if(pv.bTop) {
		o << "{PvTOP}";
		return o;
	}
	if(!pv.bTop && !pv.length()) {
		o << "{PvBOT}";
		return o;
	}

	o << "{";
	bool fst = true;
	for(PotentialValue::Iterator i(pv); i(); i++) {
		if(!fst)
			o << ", ";
		else
			fst = false;
		o << "0x" << hex(*i);
	}
	o << "}";
	return o;
}

}} // end of namespace otawa { namespace dynbranch {
