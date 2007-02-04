/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_TrivialSimulator.cpp -- TrivialSimulator class implementation.
 */

#include <otawa/sim/TrivialSimulator.h>
#include <otawa/sim/State.h>
#include <otawa/program.h>
#include <otawa/otawa.h>

namespace otawa { namespace sim {


/**
 * A trivial simulator state.
 */
class TrivialState: public State {
	// !!TODO!!
	FrameWork *fw;
	int time;
	int _cycle;
	Inst *pc;
	bool running;
public:
	TrivialState(FrameWork *framework, int _time):
	fw(framework), time(_time), _cycle(0) {
		pc = fw->start();
	}
	
	TrivialState(const TrivialState& state):
	fw(state.fw), time(state.time), _cycle(state._cycle), pc(state.pc),
	running(false) {
	}
	
	// State overload
	virtual State *clone(void) {
		return new TrivialState(*this);	
	}
	
	virtual void run(Driver& driver) {
		running = true;
		while(running) {
			pc = driver.nextInstruction(*this, pc ? pc->next() : pc);
			_cycle += time;
			running = pc;
		}
	}
	
	virtual void flush(void) {
	}
	
	virtual int cycle(void) {
		return _cycle;
	}
	
	virtual void reset(void) {
		_cycle = 0;
	}
	
	virtual void stop(void) {
		running = false;
	}	
};


/**
 * Instruction execution time. Default to 5.
 */
Identifier<int> INSTRUCTION_TIME("sim.instruction_time");


/**
 * @class TrivialSimulator
 * The trivial simulator is a simplistic simulator with a fixed execution time
 * for each instruction (defined by @ref otawa::sim::INSTRUCTION_TIME). It only
 * accepts structural simulator (@ref otawa::sim::IS_STRUCUTURAL) wtihout
 * management of memory and control (@ref otawa::sim::USE_MEMORY and
 * @ref otawa::sim::USE_CONTROL).
 */


/**
 * Build a trivial simulator.
 */
TrivialSimulator::TrivialSimulator(void)
: Simulator("trivial_simulator", Version(1, 0, 0), OTAWA_SIMULATOR_VERSION) {
}


/**
 */	
State *TrivialSimulator::instantiate(FrameWork *fw, const PropList& props) {
	if(props.get<bool>(IS_FUNCTIONAL, false))
		throw Exception(*this, "IS_FUNCTIONAL property not supported");
	if(props.get<bool>(USE_MEMORY, false))
		throw Exception(*this, "USE_MEMORY property not supported");
	if(props.get<bool>(USE_CONTROL, false))
		throw Exception(*this, "USE_CONTROL property not supported");
	int time = props.get<int>(INSTRUCTION_TIME, 5);
	assert(time > 0);
	return new TrivialState(fw, time);
}


} } // otawa::sim
