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


class MyGC;

class LUBFastStateCombineProcessor {
public:
	inline const PotentialValue process(const PotentialValue& pva, const PotentialValue& pvb) {
		// only add the values exist in both domain
		if((pva.length() == 0) || (pvb.length() == 0))
			return PotentialValue::top;
		return merge(pva, pvb); // neither pva nor pvb is empty
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


/**
 * We need to have a wrapper for FastState so that it can be used as the Domain of the AI
 * This is because FastState does not provide functions such as lub ...
 */
class FastStateWrapper {
	friend MyGC;
public:
	typedef otawa::dfa::FastState<PotentialValue, MyGC>::t state_t;
	typedef otawa::dfa::FastState<PotentialValue, MyGC>* fast_state_t;
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
#define EVALUATIONx
	inline FastStateWrapper(void) : _bottom(true), _state(0), _fastState(0) { // default constructor
	}

	inline FastStateWrapper(const FastStateWrapper& fsw){ // copy constructor
		ASSERT(fsw._fastState);
		copy(fsw);
	}

	inline ~FastStateWrapper() {
		count--;
	}

	inline void setBottom(bool b) { _bottom = b; }
	inline bool isBottom(void) const { return _bottom; }
	inline void setFastState(otawa::dfa::FastState<PotentialValue, MyGC>* fs) { _fastState = fs; }
	inline otawa::dfa::FastState<PotentialValue, MyGC>* getFastState(void) { return _fastState; }

	inline void setState(otawa::dfa::FastState<PotentialValue, MyGC>::t s) { _state = s; } // or initialize state with _fastState->bot

	void setReg(int regNum, const PotentialValue& pv);

	inline const PotentialValue& readReg(int regNum) {
		ASSERT(_fastState);
		return _fastState->get(_state, regNum);
	}

	inline void copy(const FastStateWrapper & fsw) { 	_bottom = fsw._bottom; _fastState = fsw._fastState; _state = fsw._state; }
	inline otawa::dfa::FastState<PotentialValue, MyGC>::t getState(void) const { return _state; }


	void storeMemory(address_t addr, const PotentialValue &pv);

	inline const PotentialValue& loadMemory(address_t addr) {
		return _fastState->load(_state, addr);
	}

	void lub(const FastStateWrapper & fsw);

	void widening(const FastStateWrapper & fsw);
	inline bool equals(const FastStateWrapper & fsw) const {
		return _fastState->equals(_state, fsw._state);
	}


	int collect(const MyGC* gc, int j = 0, bool show= false) const;

	void checkState(bool f = false) const;

private:
	bool _bottom;
	state_t _state;
	fast_state_t _fastState;
	static unsigned long count;
	static int j;
};

} }

#endif 	// OTAWA_DYNBRANCH_STATE_H
