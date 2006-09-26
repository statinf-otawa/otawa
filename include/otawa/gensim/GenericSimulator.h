/*
 * $Id$
 * Copyright (c) 2006, IRIT-UPS
 *
 * otawa/gensim/GenericSimulator.h -- GenericSimulator class interface
 */
#ifndef OTAWA_GENSIM_GENERIC_SIMULATOR_H
#define OTAWA_GENSIM_GENERIC_SIMULATOR_H

#include <otawa/sim/Simulator.h>
#include <otawa/sim/State.h>
#include <otawa/otawa.h>

namespace otawa { namespace gensim {

// GenericSimulator class
class GenericSimulator: public sim::Simulator {
public:
	GenericSimulator(void);
	
	// Simulator overload
	sim::State *instantiate(FrameWork *fw,
		const PropList& props = PropList::EMPTY);
};

extern GenericIdentifier<int> DEGREE;

} } // otawa::gensim

#endif // OTAWA_GENSIM_GENERIC_SIMULATOR_H
