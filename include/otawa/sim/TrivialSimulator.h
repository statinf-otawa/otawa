/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/TrivialSimulator.h -- TrivialSimulator class interface.
 */
#ifndef OTAWA_SIM_TRIVIALSIMULATOR_H
#define OTAWA_SIM_TRIVIALSIMULATOR_H

#include <otawa/sim/Simulator.h>

namespace otawa { namespace sim {

// Configuration
extern Identifier<int> INSTRUCTION_TIME;


// TrivialSimulator class
class TrivialSimulator: public Simulator {
public:
	TrivialSimulator(void);
	
	// Simulator overload
	virtual State *instantiate(WorkSpace *fw,
		const PropList& props = PropList::EMPTY);
};

} } // otawa::sim

#endif /* OTAWA_SIM_TRIVIALSIMULATOR_H */
