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

#include <otawa/prog/Inst.h>

namespace otawa {

class Bundle {
public:
	inline Bundle(Inst *inst): i(inst) { }

	inline Address address(void) const { return i->address(); }
	int size(void) const;
	inline Address topAddress(void) const { return address() + size(); }

	Inst::kind_t kind(void) const;
	Inst *target(void) const;
	void readRegSet(RegSet& set) const;
	void writeRegSet(RegSet& set) const;
	void semInsts(sem::Block& block) const;

	class Iter: public PreIterator<Iter, Inst *> {
	public:
		inline Iter(Bundle b): done(false), i(b.i) { }
		inline bool ended(void) const { return done; }
		inline Inst *item(void) const { return i; }
		inline void next(void) { done = i->isBundleEnd(); if(!done) i = i->nextInst(); }
	private:
		bool done;
		Inst *i;
	};
	inline Iter insts(void) const { return Iter(*this); }
	inline Iter operator*(void) const { return insts(); }

	inline Inst *first(void) const { return i; }
	inline operator Inst *(void) const { return first(); }

private:
	Inst *i;
};

}	// otawa

#endif /* OTAWA_CFG_BUNDLE_H_ */
