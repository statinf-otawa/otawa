/*
 *	BBTimeSimulator class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-18, IRIT UPS.
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
#ifndef OTAWA_TSIM_BBTIMESIMULATOR_H_
#define OTAWA_TSIM_BBTIMESIMULATOR_H_

#include <otawa/proc/BBProcessor.h>
#include <otawa/sim/State.h>

namespace otawa { namespace tsim {

class BBTimeSimulator: public BBProcessor {
public:
	static p::declare reg;
	BBTimeSimulator(p::declare& r = reg);

protected:
	void setup(WorkSpace *ws) override;
	void processBB(WorkSpace *fw, CFG *cfg, Block *bb) override;
	void cleanup(WorkSpace *ws) override;

private:
	sim::State *state;
};

} }		//otawa::tsim

#endif /* OTAWA_TSIM_BBTIMESIMULATOR_H_ */
