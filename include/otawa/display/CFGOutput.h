/*
 *	$Id$
 *	CFGOutput class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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
#ifndef OTAWA_CFGOUTPUT_H_
#define OTAWA_CFGOUTPUT_H_

#include <otawa/proc/CFGProcessor.h>
#include <otawa/display/AbstractDrawer.h>

namespace otawa {

class Edge;

namespace display {

class CFGOutputDecorator;

// CFGOutput class
class CFGOutput: public CFGProcessor {
	friend class CDFGOutputDeclarator;
public:
	static p::declare reg;
	CFGOutput(AbstractRegistration& _reg = reg);

	// Configuration
	static Identifier<display::kind_t> KIND;
	static Identifier<string> PATH;

	virtual void genGraphLabel(CFG *cfg, Output& out);
	virtual void genBBLabel(CFG *cfg, BasicBlock *bb, Output& out);
	virtual void genEdgeLabel(CFG *cfg, otawa::Edge *edge, Output& out);
	virtual void genBBInfo(CFG *cfg, BasicBlock *bb, Output& out);
	virtual void genEdgeInfo(CFG *cfg, otawa::Edge *edge, Output& out);

protected:
	virtual void configure(const PropList &props);
	virtual void processCFG(WorkSpace *fw, CFG *cfg);

private:
	display::kind_t kind;
	string path;
};

} }	// otawa::display

#endif /* OTAWA_CFGOUTPUT_H_ */
