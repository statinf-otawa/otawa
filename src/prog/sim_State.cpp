/*
 *	$Id$
 *	State class implementation
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

#include <otawa/sim/State.h>
#include <otawa/sim/features.h>
#include <otawa/prog/Process.h>

namespace otawa { namespace sim {

/**
 * @class State
 * This class represents a running simulator. Objects of this class may be used
 * for driving the simulation. As most methods are pure virtual,this class must
 * be derivated before being used.
 */


/**
 */
State::~State(void) {
}


/**
 * @fn State *State::clone(void);
 * Build a copy of the current simulation state.
 * @return	Simulation state copy.
 */


/**
 * @fn mode_t State::flush(void);
 * Run the simulator, preventing the instruction fetch, until the pipeline is
 * empty.
 * @return		Processor state.
 */


/**
 * @fn void State::run(Driver& driver);
 * Run the simulator with the given driver. This method stops when the driver
 * stops the simulator.
 * @param driver	Simulator driver.
 */


/**
 * @fn void State::stop(void);
 * Stop the simulator at the end of the current cycle.
 */ 


/**
 * @fn int State::cycle(void);
 * Return the number of running cycles.
 * @return	Number of cycles.
 */


/**
 * @fn void State::reset(void);
 * Reset the cycle counter.
 */


/**
 * @fn Process *State::process(void);
 * Get the process where the simulator is running in.
 * @return	Current process.
 */

} } // otawa::sim
