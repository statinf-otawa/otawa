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
#include "PotentialValue.h"

namespace otawa { namespace dynbranch {

PotentialValue PotentialValue::bot;
PotentialValue PotentialValue::top;
PotentialValue PotentialValue::DEFAULT;
unsigned int PotentialValue::MAGIC = 0;
#ifdef SAFE_MEM_ACCESS
SLList<PotentialValueMem*> PotentialValue::potentialValueCollector;
#else
SLList<PotentialValue*> PotentialValue::potentialValueCollector;
#endif

PotentialValue::PotentialValue() : Set<elm::t::uint32>() {
	magic = MAGIC;
#ifdef SAFE_MEM_ACCESS
	pvm = 0; // not going to add into potentialValueCollector...
#endif
}

PotentialValue::PotentialValue(const PotentialValue & cpv) : Set<elm::t::uint32>(cpv) {
	magic = MAGIC;
#ifdef SAFE_MEM_ACCESS
	pvm = new PotentialValueMem();
	pvm->pv = this;
	pvm->status = true;
	potentialValueCollector.add(pvm);
#else
	potentialValueCollector.add(this);
#endif
}

//PotentialValue::~PotentialValue() { ~Set<elm::t::uint32>(); memset(this, 0, sizeof(PotentialValue)); }

PotentialValue::~PotentialValue() {
	magic = 0;
#ifdef SAFE_MEM_ACCESS
	if(pvm)
		pvm->status = false;
#endif
	~Set<elm::t::uint32>();
}

PotentialValue& PotentialValue::operator=(const PotentialValue& a) {
	// need this for FastState because the memory are initialized as chars but not as the objects
	// (!)LESSON LEARNT: when calling =, the target (this) may not be initialized yet,
	// which means the capacity is 0, this is a big nono, hence we we grow the capacity with the same size of a

	// ok someone has to be dirty
	// FIXME: possible memory leakage! because we don't know 'this' is just created by
	// fast state (which has mumbo jumbo values which play bad tricks) or already existed.
	// clear the already existed potential value will cause memory leakage, say, it will make
	// the 'tab' and 'cap' (inh. from Set and then Vector) 0, and make the operator= from Vector (which then calls its
	// copy function) see there is no 'tab' and creates one, and make a new 'tab' without clearing the old one.
	// But if we do not set it to 0, the 'tab' could have some random value, such as 0x7, and cap has smaller value
	// then a.cap. the 'tab' will be used....then it leads to disaster
	// and operator= from Vector will just use the tab

	// first, we need to see if the 'tab' of this really contains something ?
	// lets use a magic number stored in the static variable of the class MAGIC
	// if the PotentialValue is created via ordinary constructor, then magic number should be MAGIC
	// if not, it will be random value (could be MAGIC too, but lets bet?), we change the MAGIC for each new allocator because
	// it is possible that allocator gets the same chunk of memory previously and the value persists.
	// if it is not MAGIC, we know it is created by faststate, then we need to initialize the content of
	// the memory. and assigned the magic number to MAGIC (just like a 'PASS' stamp!)
	// we then use the operator= from Vector to create the tab

	if(magic != MAGIC) { // valgrind will report error on reading magic as depends on uninitialized value(s), which is true :)
		memset(this, 0, sizeof(PotentialValue));
		magic = MAGIC;
		Vector<elm::t::uint32>::operator=(a);
#ifdef SAFE_MEM_ACCESS
		pvm = new PotentialValueMem();
		pvm->pv = this;
		pvm->status = true;
		potentialValueCollector.add(pvm);
#else
		potentialValueCollector.add(this);
#endif
	}
	else{
		clear();
		for(PotentialValue::Iterator ita(a); ita; ita++)
			insert(*ita);
	}
	return *this;
}

PotentialValue operator&(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita & *itb);
	return res;
}

PotentialValue operator|(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita | *itb);
	return res;
}

PotentialValue operator^(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita ^ *itb);
	return res;
}

PotentialValue operator~(const PotentialValue& a) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
			res.insert(~(*ita));
	return res;
}

PotentialValue operator+(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita + *itb);
	return res;
}

PotentialValue operator-(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita - *itb);
	return res;
}

PotentialValue operator>>(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++) {
		for(PotentialValue::Iterator itb(b); itb; itb++) {
			res.insert(*ita >> *itb);
		}
	}
	return res;
}

PotentialValue logicalShiftRight(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++) {
		for(PotentialValue::Iterator itb(b); itb; itb++) {
			res.insert((unsigned int)*ita >> *itb);
		}
	}
	return res;
}

PotentialValue operator<<(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita << *itb);
	return res;
}

PotentialValue operator||(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		for(PotentialValue::Iterator itb(b); itb; itb++)
			res.insert(*ita || *itb);
	return res;
}

PotentialValue merge(const PotentialValue& a, const PotentialValue& b) {
	PotentialValue res;
	for(PotentialValue::Iterator ita(a); ita; ita++)
		res.insert(*ita);
	for(PotentialValue::Iterator itb(b); itb; itb++) {
		res.insert(*itb);
	}
	return res;
}

bool operator==(const PotentialValue& a, const PotentialValue& b) {
	for(PotentialValue::Iterator isa(a); isa; isa++)
		if(!b.contains(*isa))
			return false;
	for(PotentialValue::Iterator isb(b); isb; isb++)
		if(!a.contains(*isb))
			return false;
	return true;
}

Output& operator<<(Output& o, PotentialValue const& pv) {
	o << "{ ";
	for(PotentialValue::Iterator i(pv); i; i++)
		o << "0x" << hex(*i) << " ";
	o << "}";
	return o;
}

}} // end of namespace otawa { namespace dynbranch {
