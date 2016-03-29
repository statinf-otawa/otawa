/*
 *	State class interface
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
#ifndef OTAWA_DYNBRANCH_STATE_H
#define OTAWA_DYNBRANCH_STATE_H
#include <elm/io/OutStream.h>
#include "PotentialValue.h"
#include <otawa/dfa/FastState.h>
#include <otawa/prog/WorkSpace.h>
using namespace elm;

namespace otawa { namespace dynbranch {

using namespace elm::io ;
using namespace elm::genstruct ;

class LUBFastStateCombineProcessor {
public:
	inline const PotentialValue process(const PotentialValue& pva, const PotentialValue& pvb) {
		// only add the values exist in both domain
		if((pva.length() == 0) || (pvb.length() == 0))
			return PotentialValue::top;
		return merge(pva, pvb);
	}
};

class WideningFastStateCombineProcessor {
public:
	inline const PotentialValue process(const PotentialValue& pva, const PotentialValue& pvb) {
		if(pva != pvb)
			return PotentialValue::top;
		else
			return pva;
	}
};

class FastStateWrapper {
public:
	typedef otawa::dfa::FastState<PotentialValue>::t state_t;
	typedef otawa::dfa::FastState<PotentialValue>* fast_state_t;
	typedef elm::t::uint32 address_t;

	inline friend Output& operator<<(Output& o, FastStateWrapper const& pv) {
		o << "{";
		if(pv._state == pv._fastState->top)
			o << "T";
		else
			pv._fastState->print(o, pv._state);
		o << "}";
		return o;
	}

	inline FastStateWrapper(void) : _fastState(0), _state(0), _bottom(true) { // default constructor
	}

	inline FastStateWrapper(const FastStateWrapper& fsw){ // copy constructor
		assert(fsw._fastState);
		copy(fsw);
	}

	inline void setBottom(bool b) { _bottom = b; }
	inline bool isBottom(void) const { return _bottom; }
	inline void setFastState(otawa::dfa::FastState<PotentialValue>* fs) { _fastState = fs; }
	inline otawa::dfa::FastState<PotentialValue>* getFastState(void) { return _fastState; }

	inline void setState(otawa::dfa::FastState<PotentialValue>::t s) { _state = s; } // or initialize state with _fastState->bot

	inline void setReg(int regNum, const PotentialValue& pv) {
		assert(_fastState);
		_state = _fastState->set(_state, regNum, pv);
		_bottom = false;
	}

	inline const PotentialValue& readReg(int regNum) {
		assert(_fastState);
		return _fastState->get(_state, regNum);
	}

	inline void copy(const FastStateWrapper & fsw) { 	_bottom = fsw._bottom; _fastState = fsw._fastState; _state = fsw._state; }
	inline otawa::dfa::FastState<PotentialValue>::t getState(void) const { return _state; }


	inline void storeMemory(address_t addr, const PotentialValue &pv) {
		_bottom = false;
		_state = _fastState->store(_state, addr, pv);
	}

	inline const PotentialValue& loadMemory(address_t addr) {
		return _fastState->load(_state, addr);
	}

	inline void lub(const FastStateWrapper & fsw) {
		LUBFastStateCombineProcessor temp;
		_state = _fastState->combine<LUBFastStateCombineProcessor>(_state, fsw._state, temp);
	}

	inline void widening(const FastStateWrapper & fsw) {
		WideningFastStateCombineProcessor temp;
		_state = _fastState->combine<WideningFastStateCombineProcessor>(_state, fsw._state, temp);
	}

	inline bool equals(const FastStateWrapper & fsw) const {
		return _fastState->equals(_state, fsw._state);
	}

private:
	bool _bottom;
	state_t _state;
	fast_state_t _fastState;
};

typedef FastStateWrapper Domain;
typedef FastStateWrapper State;

//
//extern Identifier<Vector<Pair<Address, Address> >* > DATA_IN_READ_ONLY_REGION;
//inline bool inROData(Address addr, WorkSpace* ws) {
//	Vector<Pair<Address, Address> > *rodata = DATA_IN_READ_ONLY_REGION(ws);;
//	for(Vector<Pair<Address, Address> >::Iterator i(*rodata); i; i++) {
//		if((addr >= (*i).fst) && (addr <= (*i).snd))
//			return true;
//	}
//	return false;
//}

} }

#endif 	// OTAWA_DYNBRANCH_STATE_H
