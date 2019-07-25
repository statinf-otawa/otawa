/*
 *	$Id$
 *	Trivial Simulator implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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
	WorkSpace *fw;
	int time;
	int _cycle;
	Inst *pc;
	bool running;
public:
	TrivialState(WorkSpace *framework, int _time):
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
			pc = driver.nextInstruction(*this, pc ? pc->nextInst() : pc);
			_cycle += time;
			running = pc;
		}
	}

	virtual void flush(void) { }
	virtual int cycle(void) { return _cycle; }
	virtual void reset(void) { _cycle = 0; }
	virtual void stop(void) { running = false; }
	virtual Process *process(void) { return fw->process(); }
};


/**
 * Instruction execution time. Default to 5.
 */
Identifier<int> INSTRUCTION_TIME("otawa::sim::instruction_time");


/**
 * @class TrivialSimulator
 * The trivial simulator is a simplistic simulator with a fixed execution time
 * for each instruction (defined by @ref otawa::sim::INSTRUCTION_TIME). It only
 * accepts structural simulator (@ref otawa::sim::IS_STRUCTURAL) wtihout
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
State *TrivialSimulator::instantiate(WorkSpace *fw, const PropList& props) {
	if(IS_FUNCTIONAL(props))
		throw Exception(*this, "IS_FUNCTIONAL property not supported");
	if(USE_MEMORY(props))
		throw Exception(*this, "USE_MEMORY property not supported");
	if(USE_CONTROL(props))
		throw Exception(*this, "USE_CONTROL property not supported");
	int time = INSTRUCTION_TIME(props);
	ASSERT(time > 0);
	return new TrivialState(fw, time);
}


} } // otawa::sim
