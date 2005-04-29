/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ipet_VarAssignment.cpp -- VarAssignment class implementation.
 */

#include <otawa/ipet/IPET.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ilp/Var.h>

using namespace otawa::ilp;

namespace otawa {


/**
 * @class VarAssignment
 * This processor ensures that each basic block and each edge of the CFG
 * has a variable associated with a @ref IPET::ID_Var annotation.
 */


/**
 * See @ref BBProcessor::processCFG().
 */
void VarAssignment::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	
	// Check BB
	StringBuffer buf;
	buf.print("x%lx", bb->address());
	if(!bb->get<Var *>(IPET::ID_Var, 0)) {
		String name = buf.toString();
		bb->addDeletable<Var *>(IPET::ID_Var, new Var(name));
	}
	
	// Check out edges
	for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++) {
		if(!edge->get<Var *>(IPET::ID_Var, 0)) {
			StringBuffer buf;
			buf.print("e_%lx_%lx", bb->address(), edge->target()->address());
			String name = buf.toString();
			edge->add<Var *>(IPET::ID_Var, new Var(name));
		}
	}
}

} // otawa
