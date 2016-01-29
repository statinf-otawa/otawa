/*
 *	$Id$
 *	BasicConstraintsBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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
#ifndef OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
#define OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/ipet/features.h>
#include <otawa/ilp/Constraint.h>
namespace otawa {

// External classes	
class BasicBlock;
namespace ilp {
	class System;
	class Var;
} //ilp

namespace ipet {

// BasicConstraintsBuilder class
class BasicConstraintsBuilder: public BBProcessor {
public:
	BasicConstraintsBuilder(void);

protected:
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
	virtual void configure(const PropList &props);
	
private:
	bool _explicit;
	//void addEntryConstraint(ilp::System *system, CFG *caller, Block *bb, CFG *callee, ilp::Var *var);
};

} } // otawa::ipet

#endif 	// OTAWA_IPET_BASIC_CONSTRAINTS_BUILDER_H
