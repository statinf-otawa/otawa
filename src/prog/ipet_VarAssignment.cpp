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
void VarAssignment::processBB(BasicBlock *bb) {
	
	// Check BB
	if(!bb->get<Var *>(IPET::ID_Var, 0))
		bb->addDeletable<Var *>(IPET::ID_Var, new Var());
	
	// Check out edges
	for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++) {
		if(!edge->get<Var *>(IPET::ID_Var, 0))
			edge->add<Var *>(IPET::ID_Var, new Var());
	}
}

} // otawa
