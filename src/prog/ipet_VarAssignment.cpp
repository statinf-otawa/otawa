/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	VarAssignment class implementation.
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
 * has a variable associated with a @ref ipet::VAR annotation.
 * 
 * @par Configuration
 * @li @ref ipet::EXPLICIT : use explicit name (takes more to compute but
 * provides more meaningful variable names).
 * @li @ref RECURSIVE : add function names to the explicit variable names.
 * 
 * @par Provided Features
 * @li @ref ASSIGNED_VARS_FEATURE
 */


/**
 * Perform the actual work on the given basic block.
 * @param bb	Basic block to process.
 */
void VarAssignment::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb) {
	
	// Check BB
	if(!VAR(bb)) {
		String name = "";
		if(_explicit)
			name = makeNodeVar(bb, cfg);
		VAR(bb) = new Var(name);
	}
	
	// Check out edges
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		if(!VAR(edge)) {
			String name = "";
			if(_explicit)
				name = makeEdgeVar(edge, cfg);
			VAR(edge) = new Var(name);
		}
	}
}


/**
 * Build a new variable assignment processor.
 */
VarAssignment::VarAssignment(void)
: 	BBProcessor("otawa::VarAssignment", Version(1, 0, 0)),
	_explicit(false),
	_recursive(false)
{
	provide(ASSIGNED_VARS_FEATURE);		
}


/**
 */
void VarAssignment::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = EXPLICIT(props);
	_recursive = RECURSIVE(props);
	//cout << "_explicit = " << _explicit << io::endl;
}


/**
 * Build a node variable name.
 * @param bb	Basic block to build the variable name for.
 * @param cfg	Owner CFG.
 * @return		Basic block variable name.
 */
String VarAssignment::makeNodeVar(BasicBlock *bb, CFG *cfg) {
	assert(bb);
	StringBuffer buf;
	buf << "x";
	int num = bb->number();
	if(num >= 0) {
		buf << num;
		if(_recursive)
			buf << "_" << cfg->label();
	}
	else
		buf << bb->address();
	return buf.toString();
}


/**
 * Build an edge variable name.
 * @param edge	Basic block to build the variable name for.
 * @param cfg	Owner CFG.
 * @return		Basic block variable name.
 */
String VarAssignment::makeEdgeVar(Edge *edge, CFG *cfg) {
	assert(edge);
	StringBuffer buf;
	buf << "e";
	
	// Write source
	int num = edge->source()->number();
	if(num >= 0)
		buf << num;
	else
		buf << edge->source()->address();
	buf << "_";
	
	// Write target
	if(edge->kind() == Edge::CALL) {
		if(_recursive)
			buf << "_" << cfg->label() << "___";
		if(!edge->calledCFG())
			buf << "unknown";
		else
			buf << edge->calledCFG()->label();
	}
	else {
		num = edge->target()->number();
		if(num >= 0)
			buf << num;
		else
			buf << edge->target()->address();
		if(_recursive)
			buf << "_" << cfg->label();
	}
	
	// Return result
	return buf.toString();
}


/**
 * This feature asserts that each block and each edge has a variable
 * name asserted.
 * 
 * @par Properties
 * @li @ref ipet::VAR
 */
Feature<VarAssignment> ASSIGNED_VARS_FEATURE("otawa::ipet::assigned_vars");

} } // otawa::ipet
