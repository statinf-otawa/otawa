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

using namespace elm;
using namespace otawa::ilp;

namespace otawa { namespace ipet {


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
	if(!bb->get<Var *>(VAR, 0)) {
		String name = "";
		if(_explicit)
			name = makeNodeVar(bb);
		bb->addDeletable<Var *>(VAR, new Var(name));
	}
	
	// Check out edges
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		if(!edge->get<Var *>(VAR, 0)) {
			String name = "";
			if(_explicit)
				name = makeEdgeVar(edge);
			edge->add<Var *>(VAR, new Var(name));
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


/**
 * Build a new variable assignment processor.
 * @param props		Configuration properties.
 */
VarAssignment::VarAssignment(const PropList& props)
: BBProcessor("otawa::VarAssignment", Version(1, 0, 0), props), _explicit(false) {
	init(props);
}


/**
 */
void VarAssignment::init(const PropList& props) {
	_explicit = props.get<bool>(EXPLICIT, _explicit);
}


/**
 */
void VarAssignment::configure(PropList& props) {
	init(props);
	BBProcessor::configure(props);
}


/**
 * Build a node variable name.
 * @param bb	Basic block to build the variable name for.
 * @return		Basic block variable name.
 */
String VarAssignment::makeNodeVar(BasicBlock *bb) {
	assert(bb);
	StringBuffer buf;
	buf << "x";
	int num = bb->get<int>(CFG::ID_Index, -1);
	if(num >= 0)
		buf << num;
	else
		buf << bb->address();
	return buf.toString();
}


/**
 * Build an edge variable name.
 * @param edge	Basic block to build the variable name for.
 * @return		Basic block variable name.
 */
String VarAssignment::makeEdgeVar(Edge *edge) {
	assert(edge);
	StringBuffer buf;
	buf << "e";
	
	// Write source
	int num = edge->source()->get<int>(CFG::ID_Index, -1);
	if(num >= 0)
		buf << num;
	else
		buf << edge->source()->address();
	buf << "_";
	
	// Write target
	if(edge->kind() == Edge::CALL) {
		if(!edge->calledCFG())
			buf << "unknown";
		else if(edge->calledCFG()->label())
			buf << edge->calledCFG()->label();
		else
			buf << edge->calledCFG()->address();
	}
	else {
		num = edge->target()->get<int>(CFG::ID_Index, -1);
		if(num >= 0)
			buf << num;
		else
			buf << edge->target()->address();
	}
	
	// Return result
	return buf.toString();
}

} } // otawa::ipet
