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

namespace otawa {

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
 * <p>Then, it set the object function as maximizing the following expression:</p>
 * <p>
 * 		t1 * n1 + t2 * n2 + ... + tm * nm
 * </p>
 * <p> where </p>
 * <dl>
 * 	<dt>tk</dt><dd>Time of execution of basic block k.</dd>
 *  <dt>nk</dt><dd>Count of execution if basic block k.</dd>
 * </dl>
 * 
 * <p>And, finally, put the constraint on the entry basic block of the CFG:</p>
 * <p>
 * 		n1 = 1
 * </p>
 */


/**
 * Build the constraints for the given 
 * @param system	ILP system to use.
 * @param bb		Basic block to process.
 */
void BasicConstraintsBuilder::make(ilp::System *system, BasicBlock *bb) {
	/*cout << "BasicConstraintBuilder::make(" << system << ", "
		 << bb << " (" << bb->address() << "))\n";*/
	Var *bbv = bb->use<Var *>(IPET::ID_Var);
	Constraint *cons;
	bool used;
		
	// Input constraint
	cons = system->newConstraint(Constraint::EQ);
	cons->addLeft(1, bbv);
	used = false;
	for(Iterator<Edge *> edge(bb->inEdges()); edge; edge++) {
		if(edge->kind() != Edge::CALL) {
			cons->addRight(1, edge->use<Var *>(IPET::ID_Var));
			used = true;
		}
	}
	if(!used)
		delete cons;
		
	// Output constraint
	cons = system->newConstraint(Constraint::EQ);
	cons->addLeft(1, bbv);
	used = false;
	for(Iterator<Edge *> edge(bb->outEdges()); edge; edge++)
		if(edge->kind() != Edge::CALL) {
			cons->addRight(1, edge->use<Var *>(IPET::ID_Var));
			used = true;
		}
	if(!used)
		delete cons;
}


/**
 */	
void BasicConstraintsBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	assert(cfg);

	// Look for the framework
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	if(!system) {
		system = fw->newILPSystem();
		cfg->addDeletable<System *>(IPET::ID_System, system);
	}

	// Set constraint on start BB
	BasicBlock *entry = cfg->entry();
	assert(entry);
	Constraint *cons = system->newConstraint(Constraint::EQ, 1);
	cons->add(1, entry->use<Var *>(IPET::ID_Var));
	
	// Add constraint for each basic block
	for(CFG::BBIterator bb(cfg); bb; bb++)
			make(system, bb);	
	
	// Set object function
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit())
			system->addObjectFunction(
				bb->use<int>(IPET::ID_Time),
				bb->use<Var *>(IPET::ID_Var));
};

} //otawa
