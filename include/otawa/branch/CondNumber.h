/*
 *	CondNumber processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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
#ifndef BRANCH_CONDNUMBER_H_
#define BRANCH_CONDNUMBER_H_

#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/BBProcessor.h>

#include "features.h"

namespace otawa { namespace branch {
	
using namespace elm;

// processor
class CondNumber : public otawa::BBProcessor {
public:
	static p::declare reg;
	CondNumber(void);
protected:
	virtual void processBB(otawa::WorkSpace*, CFG *cfg, Block *bb);
	virtual void setup(WorkSpace *ws);
private:
	int *current_index;
};

} }		// otawa::branch

#endif
