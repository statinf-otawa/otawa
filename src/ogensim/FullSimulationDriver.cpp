/*
 *	$Id$
 *	exegraph module implementation
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

#include <otawa/gensim/FullSimulationDriver.h>

using namespace otawa;
using namespace otawa::sim;

/**
 * @class FullSimulationDriver <FullSimulationDriver.h>
 * This class is a simulator @ref Driver that executes the complete program.
 */

/**
 * @fn FullSimulationDriver::FullSimulationDriver();
 * FullSimulationDriver constructor.
 */

/**
 */
Inst * FullSimulationDriver::nextInstruction(State& state, Inst *_inst)
{
	if (_simulation_ended)
		return 0;

	if (_inst && (_inst->address() == end_of_simulation)) {
		_simulation_ended = true;
		return 0;
	} else {
		if (!wrong_path) {
			if (_inst) {
				inst = emulator_state->execute(_inst);
				cpt++;
			} else {
				if (inst == 0) {
					_simulation_ended = true;
					return 0;
				}
				// If we are here, then this is the beginning of the simulation and we have
				// to really fetch the first instruction (inst) i.e. return inst;

				// These two lines are commented out becauese they execute the first
				// instruction without fetching it
				//inst = emulator_state->execute(inst);
				//cpt++;
			}
		}
		// If the next instruction is the end point of the simulation,
		// we must prevent it from being fetched
		if (inst->address() == end_of_simulation) {
			_simulation_ended = true;
			return 0;
		}
		// pseudo instruction are put before the beginning of basic blocks
		// we mustn't simulate such instructions, the next real instruction
		// is inst->next()
		while (inst->isPseudo())
			inst = inst->nextInst();

		return inst;
	}
}

/**
 */
bool FullSimulationDriver::PredictBranch(State &state, Inst *branch, bool pred)
{
	if (((pred == true) && (inst == branch->target())) || ((pred == false) && (inst == branch->next()))) {
		return true; // branch well predicted
	}
	// branch mispredicted
	wrong_path = true;
	return false;
}

/**
 */

void FullSimulationDriver::redirect(State &state, Inst * branch, bool direction)
{
	if (direction == true)
		inst = branch->target();
	else
		inst = branch->nextInst();
	wrong_path = false;
}

/**
 */
void FullSimulationDriver::terminateInstruction(State& state, Inst *inst)
{
}

