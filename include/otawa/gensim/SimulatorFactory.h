/*
 *	$Id$
 *	SimulatorFactory class interface
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
#ifndef OTAWA_GENSIM_SIMULATORFACTORY_H_
#define OTAWA_GENSIM_SIMULATORFACTORY_H_

#include <otawa/gensim/GenericState.h>

namespace otawa {

namespace hard { class Memory; }

namespace gensim {

// extern classes
class MemorySystem;

// SimulatorFactory class
class SimulatorFactory {
public:
	virtual MemorySystem *makeMemory(
		cstring name,
		GenericState * gen_state,
		Trace *trace,
		const hard::Memory *mem) = 0;
};

// DefaultFactory class
class DefaultFactory: public SimulatorFactory {
public:
	static DefaultFactory DEFAULT;
	virtual MemorySystem *makeMemory(
		cstring name,
		GenericState * gen_state,
		Trace *trace,
		const hard::Memory *mem);
};

// configuration property
extern Identifier<SimulatorFactory *> FACTORY;

} } // otawa::gensim

#endif // OTAWA_GENSIM_SIMULATORFACTORY_H_
