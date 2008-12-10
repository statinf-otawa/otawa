/*
 *	$Id$
 *	SimulatorFactory class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/gensim/SimulatorFactory.h>
#include <otawa/gensim/Memory.h>

namespace otawa { namespace gensim {

/**
 * @class SimulatorFactory
 * This class is used to build and customize and simulator. You have just to pass
 * it to the generic simulator building and to replace maker methods by the creation
 * of the customized simulator part. As a simulator building base, the @ref DefaultFactory
 * may be used.
 */


/**
 * @fn MemorySystem *SimulatorFactory::makeMemory(cstring name, GenericState * state, Trace *trace, const hard::Memory *mem);
 * Build the memory system of the simulator.
 * @param name	Symbolic name of the part.
 * @param state	State of the simulator.
 * @param trace	Trace system to use.
 * @param mem	Memory descriptor. 
 */ 


/**
 * @class DefaultFactory
 * Default implementation of the simulator factory.
 */


/**
 * Instance of the default simulator factory.
 */
DefaultFactory DefaultFactory::DEFAULT;


/**
 */
MemorySystem *DefaultFactory::makeMemory(
	cstring name,
	GenericState * state,
	Trace *trace,
	const hard::Memory *mem)
{
	return new MemorySystem(&name, state, trace, mem);
}

/**
 * Used to configure the GenSim simulator building by passing a
 * simulator factory.
 * @see GenericSimulator::instantiate()
 */
Identifier<SimulatorFactory *> FACTORY("otawa::gensim::FACTORY", &DefaultFactory::DEFAULT);

} } // otawa::gensim
