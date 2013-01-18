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
#include <elm/system/Path.h>
#include <elm/genstruct/HashTable.h>
#include <elm/string.h>
#include <otawa/base.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/ilp.h>

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
	
	WorkSpace *fw;
	ilp::System *system;
	elm::genstruct::HashTable<Address, BasicBlock *> bbs;
	elm::genstruct::HashTable<String, ilp::Var *> vars;
	elm::String path;

	BasicBlock *getBB(address_t addr);
	bool newBBVar(elm::CString name, address_t addr);
	bool newEdgeVar(elm::CString name, address_t src, address_t dst);
	ilp::Var *getVar(CString name);
	bool addConstraint(ExpNode *left, ilp::Constraint::comparator_t t, ExpNode *right);
	NormNode *normalize(ExpNode *node, double mult = 1);

protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
	virtual void configure(const PropList &props = PropList::EMPTY);

public:
	static Identifier<String> PATH;
	static Registration<ConstraintLoader> reg;
	ConstraintLoader(AbstractRegistration& r = reg);
};
	
} } // otawa::ipet

#endif 	// OTAWA_IPET_CONSTRAINT_LOADER_H
