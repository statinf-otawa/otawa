/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	prog/sim_State.cpp -- State class implementation.
 */

#include <otawa/sim/State.h>

namespace otawa { namespace sim {

/**
 * @enum mode_t
 * This type gives the state of the processor after the execution of the last
 * instruction.
 */


/**
 * @var mode_t NORMAL
 * Normal state.
 */


/**
 * @var mode_t EXCEPTION
 * The last instruction has caused an exception.
 */


/**
 * @var mode_t ERROR
 * The last instruction has caused a main error. Usually, the processor
 * cannot resume after an error (memory error, bad opcode and so on).
 */


/**
 * @var mode_t SLEEPING
 * Processor is sleeping waiting for interruption. Not available for any
 * processor.
 */


/**
 * @var mode HALTED
 * Processor is halted. Notavailable for any processor.
 */


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
 * @fn mode_t State::step(void);
 * Perform a cycle of simulation.
 * @return	Processor state.
 */


/**
 * @fn mode_t State::run(void);
 * Run the simulator until the state changes from normal state.
 * @return	Processor state.
 */


/**
 * @fn mode_t State::runUntil(address_t addr);
 * Run the simulator until the PC reaches the given address (the instruction
 * at the given address is not executed).
 * @param addr	Address to stop simulation at.
 * @return		Processor state.
 */


/**
 * @fn mode_t State::runUntilBranch(void);
 * Run the simulator until a branch is encountered (the branch is executed).
 */


/**
 * @fn void State::flush(void);
 * Run the simulator, preventing the instruction fetch, until the pipeline is
 * empty.
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
 * @fn address_t State::getPC(void);
 * Get the current PC, that is, the PC of the next instruction to execute.
 * @return	Current PC.
 */


/**
 * @fn void State::setPC(address_t pc);
 * Set the PC address, that is, the address of the next instruction to execute.
 */

} } // otawa::sim
