/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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

#ifndef FULL_SIMULATION_DRIVER_H
#define FULL_SIMULATION_DRIVER_H

#include <otawa/sim/Driver.h>
#include <otawa/otawa.h>

namespace otawa {

namespace sim {

// FullSimulationDriver class
class FullSimulationDriver : public Driver {
	Inst * inst;
	SimState *emulator_state;
	address_t end_of_simulation;
	bool wrong_path;
	WorkSpace *fw;
	bool _simulation_ended;
	int cpt;
public:
	inline FullSimulationDriver(WorkSpace *_fw, address_t start);
	// Driver overload
	virtual Inst *nextInstruction(State& state, Inst *inst);
	virtual void terminateInstruction(State& state, Inst *inst);
	virtual void redirect(State &state, Inst * branch, bool direction);
	virtual bool PredictBranch(State &state, Inst * branch, bool pred);
	inline int simulationEnded();
};

// FullSimulationDriver inlines
inline FullSimulationDriver::FullSimulationDriver(WorkSpace *_fw,
		address_t start) {
	fw = _fw;
	inst = fw->findInstAt(start);
	assert(inst);
	String lab("_exit");
	end_of_simulation = fw->process()->findInstAt(lab)->address();
	assert(end_of_simulation);
	wrong_path = false;
	emulator_state = fw->process()->newState();
	_simulation_ended = false;
}

inline int FullSimulationDriver::simulationEnded() {
	return _simulation_ended;
}

}
} // otawa::sim

#endif // FULL_SIMULATION_DRIVER_H
