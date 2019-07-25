/*
 *	ConstraintLoader class implementation
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
#ifndef OTAWA_IPET_CONSTRAINT_LOADER_H
#define OTAWA_IPET_CONSTRAINT_LOADER_H

#include <elm/string.h>
#include <elm/sys/Path.h>
#include <elm/data/HashMap.h>
#include <elm/string.h>
#include <otawa/base.h>
#include <otawa/cfg/CFG.h>
#include <otawa/ilp.h>
#include <otawa/proc/CFGProcessor.h>

// External class
namespace otawa {
	namespace ipet { class ConstraintLoader; }
	class ExpNode;
	class BasicBlock;
}
int ipet_parse(otawa::ipet::ConstraintLoader *);
void ipet_error(otawa::ipet::ConstraintLoader *, const char *);

namespace otawa { namespace ipet {

class NormNode;


// ConstraintLoader class
class ConstraintLoader: public CFGProcessor {
	friend int ::ipet_parse(ConstraintLoader *);
	friend void ::ipet_error(ConstraintLoader *, const char *);

public:
	static Identifier<String> PATH;
	static p::declare reg;
	ConstraintLoader(AbstractRegistration& r = reg);

protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
	virtual void configure(const PropList &props = PropList::EMPTY);

private:
	WorkSpace *fw;
	ilp::System *system;
	elm::HashMap<Address, BasicBlock *> bbs;
	elm::HashMap<String, ilp::Var *> vars;
	elm::String path;

	Block *getBB(address_t addr);
	Block *getBB(int index);
	void newBBVar(cstring name, address_t addr);
	void newBBVar(cstring name, int index);
	bool newEdgeVar(elm::CString name, address_t src, address_t dst);
	ilp::Var *getVar(CString name);
	bool addConstraint(ExpNode *left, ilp::Constraint::comparator_t t, ExpNode *right);
	NormNode *normalize(ExpNode *node, double mult = 1);

	void error(string message);
	void fatal(string message);
};

} } // otawa::ipet

#endif 	// OTAWA_IPET_CONSTRAINT_LOADER_H
