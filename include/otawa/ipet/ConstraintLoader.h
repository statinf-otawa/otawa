/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/ConstraintLoader.h -- ConstraintLoader class interface.
 */
#ifndef OTAWA_IPET_CONSTRAINT_LOADER_H
#define OTAWA_IPET_CONSTRAINT_LOADER_H

#include <assert.h>
#include <elm/string.h>
#include <elm/genstruct/HashTable.h>
#include <elm/string.h>
#include <otawa/base.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/ilp.h>

// External class
namespace otawa {
	class ConstraintLoader;
	class ExpNode;
	class BasicBlock;
	class NormNode;
}
int ipet_parse(otawa::ConstraintLoader *);
void ipet_error(otawa::ConstraintLoader *, const char *);

namespace otawa {

// ConstraintLoader class

class ConstraintLoader: public CFGProcessor {
	friend int ::ipet_parse(ConstraintLoader *);
	friend void ::ipet_error(ConstraintLoader *, const char *);
	
	FrameWork *fw;
	ilp::System *system;
	elm::genstruct::HashTable<address_t, BasicBlock *> *bbs;
	elm::genstruct::HashTable<String, ilp::Var *> *vars;
	elm::String path;

	BasicBlock *getBB(address_t addr);
	bool newBBVar(elm::CString name, address_t addr);
	bool newEdgeVar(elm::CString name, address_t src, address_t dst);
	ilp::Var *getVar(CString name);
	bool addConstraint(ExpNode *left, ilp::Constraint::comparator_t t, ExpNode *right);
	NormNode *normalize(ExpNode *node, double mult = 1);
public:
	static Identifier ID_Path;

	// CFGProcessor overload
	virtual void configure(PropList& prop);
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};
	
} // otawa

#endif 	// OTAWA_IPET_CONSTRAINT_LOADER_H
