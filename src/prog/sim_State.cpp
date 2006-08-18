/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_State.cpp -- State class implementation.
 */

#include <otawa/sim/State.h>

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

} } // otawa::sim
