/*
 *	$Id$
 *	gensim module interface
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
#ifndef OTAWA_GENSIM_GENERIC_SIMULATOR_H
#define OTAWA_GENSIM_GENERIC_SIMULATOR_H

#include <otawa/sim/Simulator.h>
#include <otawa/sim/State.h>
#include <otawa/otawa.h>
#include <otawa/prop/Identifier.h>


namespace otawa {

namespace gensim {

extern Identifier<String> TRACE_FILE_PATH;
extern Identifier<int> TRACE_LEVEL;

// Exception class
class Exception: public otawa::Exception {
public:
	Exception(const string& message = "");
};


// GenericSimulator class
class GenericSimulator : public sim::Simulator {

public:
	GenericSimulator(void);

	// Simulator overload
	sim::State *instantiate(WorkSpace *fw,
			const PropList& props = PropList::EMPTY);
};

extern Identifier<int> DEGREE;

}
} // otawa::gensim

// Plugin access
extern otawa::sim::Simulator& gensim_simulator;
extern otawa::gensim::GenericSimulator OTAWA_SIMULATOR_HOOK;

#endif // OTAWA_GENSIM_GENERIC_SIMULATOR_H
