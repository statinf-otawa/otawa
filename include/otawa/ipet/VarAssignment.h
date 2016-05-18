/*
 *	$Id$
 *	VarAssignment class interface
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
#ifndef OTAWA_IPET_VARASSIGNMENT_H
#define OTAWA_IPET_VARASSIGNMENT_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// pre-declaration
namespace ilp { class System; }

namespace ipet {

// VarAsignment class
class VarAssignment: public BBProcessor {
public:
	VarAssignment(void);
	virtual void configure(const PropList& props);
	static p::declare reg;

protected:
	virtual void processBB(WorkSpace *fw, CFG *cfg, Block *bb);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);

private:
	bool _explicit, _recursive;
	ilp::System *sys;
	String makeNodeVar(Block *bb, CFG *cfg);
	String makeEdgeVar(Edge *edge, CFG *cfg);
};

// Features
extern Identifier<String*> FORCE_NAME;

} } // otawa::ipet

#endif	// OTAWA_IPET_VARASSIGNMENT_H

