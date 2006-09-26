/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/gensim/GenericState.h -- GenericState class interface.
 */
#ifndef OTAWA_GENSIM_GENERIC_STATE_H
#define OTAWA_GENSIM_GENERIC_STATE_H

#include <otawa/gensim/GenericProcessor.h>
#include <otawa/sim/Simulator.h>
#include <otawa/sim/State.h>
#include <otawa/otawa.h>


namespace otawa { namespace gensim {

// Configuration
extern GenericIdentifier<int> INSTRUCTION_TIME;


class GenericState: public sim::State {
	friend class GenericSimulator;
	FrameWork *fw;
	int _cycle;
	GenericProcessor* processor;	
	bool running;
	void step(void);
	
	
public:
	sim::Driver *driver;

	GenericState(FrameWork *framework):
	fw(framework), _cycle(0), driver(NULL) {
	}
	
	GenericState(const GenericState& state):
	fw(state.fw), _cycle(state._cycle), driver(NULL) {
	}
	
	void init();
	
	// State overload
	virtual State *clone(void) {
		return new GenericState(*this);	
	}
		
	virtual void run(sim::Driver& driver) {
		this->driver = &driver;
		running = true;
		while(running)
			step(); 
	}
	
	virtual void stop(void) {
		running = false;
	}
	
	virtual void flush(void) {
	}
	
	virtual int cycle(void) {
		return _cycle;
	}
	
	virtual void reset(void) {
		_cycle = 0;
	}
};

} } // otawa::gensim


#endif // OTAWA_GENSIM_GENERIC_STATE_H
