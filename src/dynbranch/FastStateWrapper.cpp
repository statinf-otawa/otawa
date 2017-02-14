#include "PotentialValue.h"
#include "State.h"
#include "GlobalAnalysisProblem.h"
#include "GC.h"

namespace otawa { namespace dynbranch {

void FastStateWrapper::setReg(int regNum, const PotentialValue& pv) {
	ASSERT(_fastState);
	_state = _fastState->set(_state, regNum, pv);
	_bottom = false;
}

void FastStateWrapper::lub(const FastStateWrapper & fsw) {
	LUBFastStateCombineProcessor temp;
	_state = _fastState->combine<LUBFastStateCombineProcessor>(_state, fsw._state, temp);
}

void FastStateWrapper::widening(const FastStateWrapper & fsw) {
	WideningFastStateCombineProcessor temp;
	_state = _fastState->combine<WideningFastStateCombineProcessor>(_state, fsw._state, temp);
}


void FastStateWrapper::storeMemory(address_t addr, const PotentialValue &pv) {
	_bottom = false;
	_state = _fastState->store(_state, addr, pv);
}

int FastStateWrapper::collect(const MyGC*  gc, int j, bool show) const {
	return _fastState->collect(_state, j, show);
}


unsigned long FastStateWrapper::count = 0;
int FastStateWrapper::j = 0;

}};
