#ifndef _GenericSIMULATOR_H_
#define _GenericSIMULATOR_H_

/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/TrivialSimulator.h -- TrivialSimulator class interface.
 */

#include <otawa/sim/Simulator.h>
#include <otawa/sim/State.h>
#include <otawa/otawa.h>

namespace otawa { 

// GenericSimulator class
class GenericSimulator: public sim::Simulator {
public:
	GenericSimulator(void);
	
	// Simulator overload
	sim::State *instantiate(FrameWork *fw,
		const PropList& props = PropList::EMPTY);
};

extern GenericIdentifier<int> DEGREE;

} // otawa

#endif //_GenericSIMULATOR_H_
