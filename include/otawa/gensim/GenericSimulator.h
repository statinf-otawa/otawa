#ifndef _GenericSIMULATOR_H_
#define _GenericSIMULATOR_H_

/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/TrivialSimulator.h -- TrivialSimulator class interface.
 */

#include <otawa/gensim/GenericProcessor.h>
#include <otawa/sim/Simulator.h>
#include <otawa/sim/State.h>
#include <otawa/otawa.h>


namespace otawa { 

// Configuration
extern GenericIdentifier<int> INSTRUCTION_TIME;


class GenericState: public sim::State {
	FrameWork *fw;
	int time;
	int _cycle;
	Inst* next;
	GenericProcessor* processor;	
	bool running;
	void step(void);
	
	
public:
	sim::Driver *driver;

	GenericState(FrameWork *framework, int _time):
	fw(framework), time(_time), _cycle(0), driver(NULL) {
		next = fw->start();
	}
	
	GenericState(const GenericState& state):
	fw(state.fw), time(state.time), _cycle(state._cycle), next(state.next), driver(NULL) {
	}
	
	void init();
	
	// State overload
	virtual State *clone(void) {
		return new GenericState(*this);	
	}
		
	virtual void run(sim::Driver& driver) {
		next = driver.firstInstruction(*this);
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

// GenericSimulator class
class GenericSimulator: public sim::Simulator {
public:
	GenericSimulator(void);
	
	// Simulator overload
	GenericState *instantiate(FrameWork *fw,
		const PropList& props = PropList::EMPTY);
};




} // otawa


#endif //_GenericSIMULATOR_H_
