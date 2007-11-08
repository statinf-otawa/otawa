/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ipet_BasicConstraintsBuilder.h -- BasicConstraintsBuilder class implementation.
 */

#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/cfg.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/ILPSystemGetter.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

/**
 * Used to record the constraint of a called CFG.
 */
Identifier<Constraint *> CALLING_CONSTRAINT("otawa::ipet::calling_constraint", 0);


/**
 * @class BaseConstraintsBuilder
 * <p>This code processor create or re-use the ILP system in the framework and
 * add to it basic IPET system constraints as described in the article below:</p>
 * <p>Y-T. S. Li, S. Malik, <i>Performance analysis of embedded software using
 * implicit path enumeration</i>, Workshop on languages, compilers, and tools
 * for real-time systems, 1995.</p>
 * 
 * <p>For each basic bloc, the following
 * constraint is added:</p>
 * <p>
 * 		ni = i1 + i2 + ... + in<br>
 * 		ni = o1 + o2 + ... + om
 * </p>
 * <p> where </p>
 * <dl>
 * 	<dt>ni</dt><dd>Count of executions of basic block i.</dd>
 *  <dt>ik</dt><dd>Count of traversal of input edge k in basic block i</dd>
 *  <dt>ok</dt><dd>Count of traversal of output edge k in basic block i</dd>
 * </dl>
 * 
 * <p>And, finally, put the constraint on the entry basic block of the CFG:</p>
 * <p>
 * 		n1 = 1
 * </p>
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
void BasicConstraintsBuilder::processBB (WorkSpace *fw, CFG *cfg, BasicBlock *bb)
{
	assert(fw);
	assert(cfg);
	assert(bb);

	// Prepare data
	Constraint *cons;
	bool used;
	CFG *called = 0;
	System *system = SYSTEM(fw);
	assert(system);
	Var *bbv = VAR( bb);
		
	// Input constraint
	cons = system->newConstraint(Constraint::EQ);
	cons->addLeft(1, bbv);
	used = false;
	for(BasicBlock::InIterator edge(bb); edge; edge++)
		if(edge->kind() != Edge::CALL) {
			cons->addRight(1, VAR(edge));
			used = true;
		}
	if(!used)
		delete cons;
	
	// Output constraint
	cons = system->newConstraint(Constraint::EQ);
	cons->addLeft(1, bbv);
	used = false;
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		if(edge->kind() != Edge::CALL) {
			cons->addRight(1, VAR(edge));
			used = true;
		}
		else
			called = edge->calledCFG();
	}
	if(!used)
		delete cons;

	// Process the call
	if(called) {
		cons = CALLING_CONSTRAINT(called);
		if(!cons) {
			cons = system->newConstraint(Constraint::EQ);
			assert(cons);
			cons->addLeft(1, VAR(called->entry()));
			CALLING_CONSTRAINT(called) = cons;
		}
		cons->addRight(1, bbv);
	}	
}


/**
 */	
void BasicConstraintsBuilder::processWorkSpace(WorkSpace *fw) {
	assert(fw);
	
	// Call the orignal processing
	BBProcessor::processWorkSpace(fw);
	
	// Just record the constraint "entry = 1"
	CFG *cfg = ENTRY_CFG(fw);
	assert(cfg);
	System *system = SYSTEM(fw);
	BasicBlock *entry = cfg->entry();
	assert(entry);
	Constraint *cons = system->newConstraint(Constraint::EQ, 1);
	cons->addLeft(1, VAR(entry));
};


/**
 * Build basic constraint builder processor.
 */
BasicConstraintsBuilder::BasicConstraintsBuilder(void)
: BBProcessor("otawa::ipet::BasicConstraintsBuilder", Version(1, 0, 0)) {
	provide(CONTROL_CONSTRAINTS_FEATURE);
	require(ASSIGNED_VARS_FEATURE);
	require(ILP_SYSTEM_FEATURE);
}


/**
 * This feature ensures that control constraints has been added to the
 * current ILP system.
 */
Feature <BasicConstraintsBuilder>
	CONTROL_CONSTRAINTS_FEATURE("otawa::control_constraints");

} } //otawa::ipet
