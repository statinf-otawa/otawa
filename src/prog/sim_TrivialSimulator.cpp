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
	FrameWork *fw;
	int time;
	int _cycle;
	Inst *next;
public:
	TrivialState(FrameWork *framework, int _time):
	fw(framework), time(_time), _cycle(0) {
		next = fw->start();
	}
	
	TrivialState(const TrivialState& state):
	fw(state.fw), time(state.time), _cycle(state._cycle), next(state.next) {
	}
	
	// State overload
	virtual State *clone(void) {
		return new TrivialState(*this);	
	}
	
	virtual mode_t step(void) {
		if(!next)
			return ERROR;
		_cycle += time;
		next = next->next();
		return NORMAL;
	}
	
	virtual mode_t run(void) {
		mode_t mode = NORMAL;
		while(mode == NORMAL)
			mode = step();
		return mode;
	}
	
	virtual mode_t runUntil(address_t addr) {
		mode_t mode = NORMAL;
		while(mode == NORMAL && next->address() != addr)
			mode = step();
		return mode;
	}

	virtual mode_t runUntil(Inst *inst) {
		assert(inst);
		return runUntil(inst->address());
	}

	virtual mode_t runUntilBranch(void) {
		mode_t mode = NORMAL;
		bool stop = false;
		while(mode == NORMAL && !stop) {
			if(next)
				stop = next->isBranch();
			mode = step();
		}
		return mode;
	}
	
	virtual mode_t flush(void) {
		return NORMAL;
	}
	
	virtual int cycle(void) {
		return _cycle;
	}
	
	virtual void reset(void) {
		_cycle = 0;
	}
	
	virtual address_t getPC(void) {
		assert(next);
		return next->address();
	}

	virtual Inst *pcInst(void) {
		return next;
	}

	virtual void setPC(address_t pc) {
		next = fw->findInstAt(pc);
	}
	
	virtual void setPC(Inst *inst) {
		assert(inst);
		next = inst;
	}
	
};


/**
 * Instruction execution time. Default to 5.
 */
GenericIdentifier<int> INSTRUCTION_TIME("sim.instruction_time");


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
: Simulator("trivial_simulator", Version(1, 0, 0),
	"A simplistic trivial simulator for any architecture.") {
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
