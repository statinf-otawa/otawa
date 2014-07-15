/*
 *	Copyright (c) 2006-07, IRIT UPS.
 *
 *	BBTimeSimulator class interface
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_TSIM_BBTIMESIMULATOR_H
#define OTAWA_TSIM_BBTIMESIMULATOR_H

#include <otawa/cfg.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa {

// Predeclaration
namespace sim {
	class State;
}

namespace tsim {

// BBTimeSimulator class
class BBTimeSimulator : public BBProcessor {
public:
	BBTimeSimulator(void);
	static Registration<BBTimeSimulator> reg;

protected:
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
private:
	sim::State *state;
};

} }	// otawa::tsim

#endif /* OTAWA_TSIM_BBTIMESIMULATOR_H */
