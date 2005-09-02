/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_IPET.cpp -- IPET class implementation.
 */

#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/manager.h>

namespace otawa { namespace ipet {

using namespace ilp;
 
/**
 * @class IPET
 * This static class is used for storing ressources (property identifiers)
 * used by IPET processors.
 */


/**
 * This identifier is used for storing the time of execution in cycles (int)
 * of the program area it applies to.
 */
Identifier IPET::ID_Time("ipet.time");


/**
 * This identifier is used for storing in basic blocks and edges the variables
 * (otawa::ilp::Var *) used in ILP resolution.
 */
Identifier IPET::ID_Var("ipet.var");


/**
 * Identifier of annotations used for storing ILP system (otawa::ilp::System *)
 * in the CFG object.
 */
Identifier IPET::ID_System("ipet.system");


/**
 * Identifier of annotation used for storing for storing the WCET value (int)
 * in the CFG of the computed function.
 */
Identifier IPET::ID_WCET("ipet.wcet");


/**
 * Identifier of a boolean property requiring that explicit names must be used.
 * The generation of explicit names for variables may be time-consuming and
 * must only be activated for debugging purposes.
 */
Identifier IPET::ID_Explicit("ipet.explicit");


/**
 * Get the system tied with the given CFG. If none exists, create ones.
 * @param fw	Current framework.
 * @param cfg	Current CFG.
 * @preturn		CFG ILP system.
 */
ilp::System *IPET::getSystem(FrameWork *fw, CFG *cfg) {
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	if(!system) {
		system = fw->newILPSystem();
		cfg->addDeletable<System *>(IPET::ID_System, system);
	}
	return system;
}


/**
 * Get the variable tied to the given basic block. If none is tied, creates a
 * new one and ties it.
 * @param system	Current ILP system.
 * @param bb		Looked basic block.
 * @return			Tied variable.
 */
ilp::Var *IPET::getVar(ilp::System *system, BasicBlock *bb) {
	Var *var = bb->get<Var *>(IPET::ID_Var, 0);
	if(!var) {
		var = system->newVar();
		bb->add(IPET::ID_Var, var);
	}
	return var;
}

/**
 * Get the variable tied to the given edge. If none is tied, creates a
 * new one and ties it.
 * @param system	Current ILP system.
 * @param edge		Looked edge.
 * @return			Tied variable.
 */
ilp::Var *IPET::getVar(ilp::System *system, Edge *edge) {
	Var *var = edge->get<Var *>(IPET::ID_Var, 0);
	if(!var) {
		var = system->newVar();
		edge->add(IPET::ID_Var, var);
	}
	return var;
}

} } // otawa::ipet
