/**
 * $Id$
 * Copyright (c) 2007, IRIT - UPS <casse@irit.fr>
 *
 * Star12X class declarations
 * This file is part of OTAWA
 *
 * OTAWA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * OTAWA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_S12X_H
#define OTAWA_S12X_H

#include <otawa/loader/old_gliss/Process.h>
#include <otawa/prog/Loader.h>
#include <otawa/sim/Simulator.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/sim/State.h>
#include <otawa/loader/s12x.h>

namespace otawa { namespace s12x {
	
// Process class
class Process: public otawa::loader::old_gliss::Process {
public:
	Process(
		Manager *manager,
		otawa::Loader *loader,
		hard::Platform *pf,
		const PropList& props = PropList::EMPTY);
	virtual int instSize(void) const { return 0; }
	int computeSize(address_t addr);
	virtual sim::Simulator *simulator(void);
	long stackChange(Inst *inst);
	unsigned long stackAccess(Inst *inst);

protected:
	virtual otawa::Inst *decode(address_t addr);
	virtual void *memory(void);
};

// otawa::gliss::Loader class
class Loader: public otawa::Loader {
public:
	Loader(void);

	// otawa::Loader overload
	virtual CString getName(void) const;
	virtual otawa::Process *load(Manager *_man, CString path, const PropList& props);
	virtual otawa::Process *create(Manager *_man, const PropList& props);
};

// Simulator class
class Simulator: public otawa::sim::Simulator {
public:
	Simulator(void);
	virtual sim::State *instantiate(WorkSpace *ws, const PropList& props);
};
extern Simulator simulator;

} } // otawa::s12x

extern otawa::s12x::Loader& s12x_plugin;

#endif	// OTAWA_S12X_H
