/*
 *	Bundle class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_CFG_BUNDLE_H_
#define OTAWA_CFG_BUNDLE_H_

#include <elm/data/Range.h>
#include <otawa/prog/Inst.h>

namespace otawa {

template <class I>
class BaseBundle {
public:
	inline BaseBundle() {}
	inline BaseBundle(const BaseBundle& b) { f = b.f; }
	inline BaseBundle(const I& i): f(i) { }

	inline Address address(void) const { return f->address(); }

	inline int size() const
		{ int s = 0; for(I i(f); i; i++) s += i->size(); return s; }

	inline Address topAddress(void) const { return address() + size(); }

	inline Inst::kind_t kind(void) const
		{ Inst::kind_t k = 0; for(I i(f); i(); i++) k |= i->kind(); return k; }

	Inst *target(void) const
	 { for(I i(f); i; i++) if(i->isBranch()) {
		 if(i->isIndirect()) return null<Inst>(); else return i->target(); } return null<Inst>(); }

	void readRegSet(RegSet& set) const
	 	 { for(I i(f); i; i++) i->readRegSet(set); }

	void writeRegSet(RegSet& set) const
		{ for(I i(f); i; i++) i->writeRegSet(set); }

	void semInsts(sem::Block& block) const
		{ int tmp = -1; for(I i(f); i(); i++) tmp -= i->semInsts(block, tmp);
		  tmp = -1; for(I i(f); i(); i++) tmp -= i->semWriteBack(block, tmp); }

	void semKernel(sem::Block& block) const
		{ int tmp = -1; for(I i(f); i(); i++) tmp -= i->semKernel(block, tmp);
		  tmp = -1; for(I i(f); i(); i++) tmp -= i->semWriteBack(block, tmp); }

	typedef I Iter;
	inline const I& iter() const { return f; }
	inline Range<I> insts(void) const { return Range<I>(f, I()); }
	inline Range<I> operator*(void) const { return insts(); }

	inline Inst *first(void) const { return *f; }
	inline operator Inst *(void) const { return *f; }

private:
	I f;
};

/*class BundleInstIter: public PreIterator<BundleInstIter, Inst *> {
public:
	inline BundleInstIter(): done(true), i(nullptr) { }
	inline BundleInstIter(Inst *_i): done(false), i(_i) { }
	inline bool ended(void) const { return done; }
	inline Inst *item(void) const { return i; }
	inline void next(void) { done = i->isBundleEnd(); if(!done) i = i->nextInst(); }
private:
	bool done;
	Inst *i;
};

class Bundle: public BaseBundle<BundleInstIter> {
public:
	inline Bundle(Inst *i): BaseBundle<BundleInstIter>(i) { };
};*/

}	// otawa

#endif /* OTAWA_CFG_BUNDLE_H_ */
