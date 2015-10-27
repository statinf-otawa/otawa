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
 * First, the entry node e_f of function f executes as many times as it is called:
 *
 * 		x_i = \sum{(i, f) in called_i} x_i,f
 *
 * where
 * @li called_i -- edges performing a call from basic block i,
 * @li x_i,f -- execution occurrences of edges calling f from i.
 *
 * Then, a work-around must be introduced to support OTAWA CFG implementation
 * that handles in parallel returning edges and calling edges. To maintain
 * consistency, the no-taken edge of a calling BB is considered as the number of times
 * the call is returning.
 *
 * If we have an uncondtional call with not-taken edge (i, k), we have the following constraints:
 *
 * 		x_i = \sum{(i, j) in E \ calling_i } x_j,i
 *		x_i,k = \sum{(i, f) in calling_i} x_i,f
 *
 *	where calling_i = { set of calling edges of i }
 *
 *  If i is conditional call BB (meaning the not-taken edge (i, k) maybe taken without a call),
 *  we get:
 *
 *  	x_i = \sum{(i, j) in E \ calling_i } x_j,i
 *  	x_i,k >= \sum{(i, f) in calling_i} x_i,f
 *
 * @par Provided Features
 * @li @ref ipet::CONTROL_CONSTRAINTS_FEATURE
 *
 * @par Required Features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 */


/**
 * Add the given variable to the entry constraint of the given CFG.
 * @param system	Current ILP system.
 * @param called	Looked CFG.
 * @param var		Variable to add.
 * @return			Entry CFG constraint.
 */
void BasicConstraintsBuilder::addEntryConstraint(System *system, CFG *cfg, BasicBlock *bb, CFG *called, Var *var) {
	static string label = "call constraint";

	// compute incrementally
	//		x_entry^f = \sum x_call^f
	model m(system);
	cons c;
	if(!called->hasProp(CALLING_CONSTRAINT)) {
		c = m(label) + x(VAR(called->entry())) == 0.;
		CALLING_CONSTRAINT(called) = &c;
	}
	else
		c = *CALLING_CONSTRAINT(called);
	c += x(var);
}


/**
 */
void BasicConstraintsBuilder::processBB (WorkSpace *fw, CFG *cfg, BasicBlock *bb)
{
	static string 	input_label = "structural input constraint",
					output_label = "structural output constraint",
					return_label = "return constraint";
	ASSERT(fw);
	ASSERT(cfg);
	ASSERT(bb);

	// Prepare data
	//Constraint *cons;
	//bool used = false;
	model m(SYSTEM(fw));
	var bbv(VAR(bb));

	// input constraint (call input on entry node are ignored)
	//		x_i = \sum{(j, i) in E /\ not call (j, i)} x_j,i (a)
	if(!bb->isEntry()) {
		cons c = m(input_label) + bbv == 0.;
		for(BasicBlock::InIterator edge(bb); edge; edge++)
			if(edge->kind() != Edge::CALL)
				c += x(VAR(edge));
	}

	// Output constraint (why separating call from other and specially many calls?)
	//		x_i = \sum{(i, j) in E /\ not call (i, j)} x_i,j
	bool is_call = false;
	Edge *nt = 0;
	if(!bb->isExit()) {
		cons c = m(output_label) + bbv == 0.;
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			if(edge->kind() != Edge::CALL) {
				c += x(VAR(edge));
				// VIRTUAL has been to support case of non-call branch at end of CFG
				if(edge->kind() == Edge::NOT_TAKEN || edge->kind() == Edge::VIRTUAL)
					nt = edge;
			}
			else
				is_call = true;
	}


	// process calls if any
	//		if not conditional	x_nt = 	sum{(i,f) in calling_i} x_if
	//		if conditional		x_nt >= sum{(i,f) in calling_i} x_if
	if(is_call) {
		cons c;
		if(bb->isConditional())
			c = m(return_label) + x(VAR(nt)) >= 0.;
		else
			c = m(return_label) + x(VAR(nt)) == 0.;
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			if(edge->kind() == Edge::CALL) {
				CFG *called = edge->calledCFG();
				if(!called)
					throw ProcessorException(*this, _ << "unresolved call at " << bb->address());
				addEntryConstraint(m, cfg, bb, called, VAR(edge));
				c += x(VAR(edge));
			}
	}

}

/**
 */
void BasicConstraintsBuilder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);
	static string label = "program entry constraint";
	CFG *cfg = ENTRY_CFG(fw);
	ASSERT(cfg);
	BasicBlock *entry = cfg->entry();
	ASSERT(entry);

	// Call the orignal processing
	BBProcessor::processWorkSpace(fw);

	// initial constraint
	//		x_e = 1
	model m(SYSTEM(fw));
	cons c = m(label) + x(VAR(entry)) == 1;
	CALLING_CONSTRAINT(cfg) = &c;
};


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
