/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ipet_VarAssignment.cpp -- VarAssignment class implementation.
 */

#include <otawa/ipet/IPET.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ilp/Var.h>
#include <otawa/cfg.h>

using namespace otawa::ilp;

namespace otawa {


/**
 * @class VarAssignment
 * This processor ensures that each basic block and each edge of the CFG
 * has a variable associated with a @ref IPET::ID_Var annotation.
 */


/**
 * Perform the actual work on the given basic block.
 * @param bb	Basic block to process.
 */
void VarAssignment::process(BasicBlock *bb) {
	//cout << "VarAssignment::process(" << bb->address() << ")\n";
	
	// Check BB
	StringBuffer buf;
	buf.print("x%lx", bb->address());
	if(!bb->get<Var *>(IPET::ID_Var, 0)) {
		String name = buf.toString();
		bb->addDeletable<Var *>(IPET::ID_Var, new Var(name));
	}
	
	// Check out edges
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		if(!edge->get<Var *>(IPET::ID_Var, 0)) {
			StringBuffer buf;
			buf.print("e_%lx_%lx", bb->address(), edge->target()->address());
			String name = buf.toString();
			edge->add<Var *>(IPET::ID_Var, new Var(name));
		}
	}
}


/**
 * See @ref CFGProcessor::processCFG().
 */
void VarAssignment::processCFG(FrameWork *fw, CFG *cfg) {
	BBProcessor::processCFG(fw, cfg);
}


/**
 * See @ref BBProcessor::processBB().
 */
void VarAssignment::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	process(bb);
}

} // otawa
