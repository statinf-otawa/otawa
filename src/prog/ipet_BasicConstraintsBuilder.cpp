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

using namespace otawa::ilp;

namespace otawa { namespace ipet {

/**
 * Used to record the constraint of a called CFG.
 */
GenericIdentifier<Constraint *> CALLING_CONSTRAINT("", 0);


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
 */


/**
 */
void BasicConstraintsBuilder::processBB (FrameWork *fw, CFG *cfg, BasicBlock *bb)
{
	assert(fw);
	assert(cfg);
	assert(bb);

	// Prepare data
	Constraint *cons;
	bool used;
	CFG *called = 0;
	System *system = getSystem(fw, ENTRY_CFG(fw));
	assert(system);
	Var *bbv = getVar(system, bb);
		
	// Input constraint
	cons = system->newConstraint(Constraint::EQ);
	cons->addLeft(1, bbv);
	used = false;
	for(BasicBlock::InIterator edge(bb); edge; edge++)
		if(edge->kind() != Edge::CALL) {
			cons->addRight(1, getVar(system, edge));
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
			cons->addRight(1, getVar(system, edge));
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
			cons->addLeft(1, getVar(system, called->entry()));
			CALLING_CONSTRAINT(called) = cons;
		}
		cons->addRight(1, bbv);
	}	
}


/**
 */	
void BasicConstraintsBuilder::processFrameWork(FrameWork *fw) {
	assert(fw);
	
	// Call the orignal processing
	BBProcessor::processFrameWork(fw);
	
	// Just record the constraint "entry = 1"
	CFG *cfg = ENTRY_CFG(fw);
	assert(cfg);
	System *system = getSystem(fw, cfg);
	BasicBlock *entry = cfg->entry();
	assert(entry);
	Constraint *cons = system->newConstraint(Constraint::EQ, 1);
	cons->addLeft(1, getVar(system, entry));
};


/**
 * Build basic constraint builder processor.
 * @param props	Configuration properties.
 */
BasicConstraintsBuilder::BasicConstraintsBuilder(const PropList& props)
: BBProcessor("otawa::BasicConstraintsBuilder", Version(1, 0, 0), props) {
}

} } //otawa::ipet
