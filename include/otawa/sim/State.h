/*
 *	$Id$
 *	State class interface
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
#ifndef OTAWA_SIM_STATE_H
#define OTAWA_SIM_STATE_H

#include <otawa/base.h>
#include <otawa/sim/Driver.h>
#include <otawa/prop/PropList.h>

namespace otawa {

// External classes
class Inst;
class Process;
	
namespace sim {

// State class
class State: public PropList {
protected:
	virtual ~State(void);
public:
	virtual State *clone(void) = 0;
	virtual void run(Driver& driver) = 0;
	virtual void stop(void) = 0;
	virtual void flush(void) = 0;
	virtual int cycle(void) = 0;
	virtual void reset(void) = 0;
	virtual Process *process(void) = 0;
};

} } // otawa::sim
	
#endif /* OTAWA_SIM_STATE_H */
