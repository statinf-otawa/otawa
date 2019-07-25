/*
 *	BBPathDriver class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#ifndef OTAWA_SIM_BBPATHDRIVER_H
#define OTAWA_SIM_BBPATHDRIVER_H

#include <otawa/cfg.h>
#include <otawa/tsim/BBPath.h>
#include <otawa/sim/State.h>

namespace otawa { namespace sim {

class InstIterator;

class BBPathDriver: public Driver {
	tsim::BBPath::BBIterator bb_iter;
	BasicBlock::InstIter *inst_iter;
	bool ended;
public:
	BBPathDriver(tsim::BBPath& bbpath);
	virtual ~BBPathDriver();
	virtual Inst* firstInstruction(State &state);
	virtual Inst* nextInstruction(State &state, Inst *inst);
	virtual void terminateInstruction(State &state, Inst *inst);
};

} }

#endif /* OTAWA_SIM_BBPATHDRIVER_H*/
