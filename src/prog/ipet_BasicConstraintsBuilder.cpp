/*
 *	$Id$
 *	BasicConstraintsBuilder class implementation
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

#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/cfg.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/ilp/expr.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

/**
 * Used to record the constraint of a called CFG.
 */
Identifier<Constraint *> CALLING_CONSTRAINT("otawa::ipet::CALLING_CONSTRAINT", 0);


/**
 * @class BaseConstraintsBuilder
 * This code processor create or re-use the ILP system in the framework and
 * add to it basic IPET system constraints as described in the article below:
 *
 * Y-T. S. Li, S. Malik, <i>Performance analysis of embedded software using
 * implicit path enumeration</i>, Workshop on languages, compilers, and tools
 * for real-time systems, 1995.
 *
 * In a CFG G=(V, E), for each basic bloc_i, the following
 * constraints are added:</p>
 *
 * 		x_i = \sum{(j, i) in E} x_j,i<br>
 * 		ni = \sum{(i, j) in E} x_i,j
 *
 * where
 * 	@li x_i -- Count of executions of basic block i,
 *  @li x_j,i -- Count of traversal of input edge (j,i) in basic block i,
 *  @li x_i,j -- Count of traversal of output edge (i,j) in basic block i.

 *
 * In addition, put the constraint on the entry basic block e of the CFG:</p>
 *
 * 		x_e = 1
 *
 * There are a special processing for basic block performing a function call.
 * First, the entry node x_e of function f executes as many times as it is called:
 *
 * 		x_e = \sum{x_c in called_i} x_c
 *
 * where
 * @li called_i -- synthetic blocks performing a call to f,
 * @li x_f -- execution frequency of synthetic block calling f.
 *
 * @par Provided Features
 * @li @ref ipet::CONTROL_CONSTRAINTS_FEATURE
 *
 * @par Required Features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 */


/**
 */
void BasicConstraintsBuilder::processCFG(WorkSpace *ws, CFG *cfg) {
	static string entry_label = "program entry constraint";
	static string call_label = "call constraint";
	model m(SYSTEM(ws));
	Block *entry = cfg->entry();

	// entry CFG:
	//		x_e = 1
	if(cfg == ENTRY_CFG(ws)) {
		ASSERT(entry);
		m(entry_label) + x(VAR(entry)) == 1;
	}

	// call case
	//	x_e = sum{i in caller(f)} x_i
	else {
		cons c = m(call_label) + x(VAR(entry)) == 0.;
		for(auto call: cfg->callers())
			c += x(VAR(call));
	}

	// call parent
	BBProcessor::processCFG(ws, cfg);
}


/**
 */
void BasicConstraintsBuilder::processBB (WorkSpace *fw, CFG *cfg, Block *bb)
{
	static string 	input_label = "structural input constraint",
					output_label = "structural output constraint",
					return_label = "return constraint";
	ASSERT(fw);
	ASSERT(cfg);
	ASSERT(bb);

	// Prepare data
	model m(SYSTEM(fw));
	var bbv(VAR(bb));

	// input constraint (call input on entry node are ignored)
	//		x_i = \sum{(j, i) in E /\ not call (j, i)} x_j,i (a)
	if(!bb->isEntry()) {
		cons c = m(input_label) + bbv == 0.;
		for(Block::EdgeIter edge = bb->ins(); edge(); edge++)
				c += x(VAR(*edge));
	}

	// output constraint (why separating call from other and specially many calls?)
	//		x_i = \sum{(i, j) in E /\ not call (i, j)} x_i,j
	if(!bb->isExit()) {
		cons c = m(output_label) + bbv == 0.;
		for(Block::EdgeIter edge = bb->outs(); edge(); edge++)
			c += x(VAR(*edge));
	}
}


/**
 * Build basic constraint builder processor.
 */
BasicConstraintsBuilder::BasicConstraintsBuilder(void)
: BBProcessor("otawa::ipet::BasicConstraintsBuilder", Version(1, 0, 0)), _explicit(false) {
	provide(CONTROL_CONSTRAINTS_FEATURE);
	require(ASSIGNED_VARS_FEATURE);
	require(ILP_SYSTEM_FEATURE);
}


void BasicConstraintsBuilder::configure(const PropList &props) {
	BBProcessor::configure(props);
	_explicit = EXPLICIT(props);
}


/**
 * This feature ensures that control constraints has been added to the
 * current ILP system.
 *
 * @par Properties
 * @li otawa::ipet::CONTROL_CONSTRAINTS_FEATURE
 */
p::feature CONTROL_CONSTRAINTS_FEATURE("otawa::ipet::CONTROL_CONSTRAINTS_FEATURE", new Maker<BasicConstraintsBuilder>());

} } //otawa::ipet
