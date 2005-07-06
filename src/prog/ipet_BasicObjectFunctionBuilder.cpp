/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicConstraintsBuilder.h -- BasicConstraintsBuilder class implementation.
 */


#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>

using namespace otawa::ilp;

namespace otawa {

/**
 * @class BasicObjectFunctionBuilder
 * This processor is used for building the basic object function function
 * to maximize for resolving the IPET system, that is,
 * @p Then, it set the object function as maximizing the following expression:
 * @p	t1 * n1 + t2 * n2 + ... + tm * nm
 * @p where
 * <dl>
 * 	<dt>tk</dt><dd>Time of execution of basic block k.</dd>
 *  <dt>nk</dt><dd>Count of execution if basic block k.</dd>
 * </dl>
 * 
 */


/**
 */
void BasicObjectFunctionBuilder::processCFG(FrameWork *fw, CFG *cfg) {

	// Look for the framework
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	if(!system) {
		system = fw->newILPSystem();
		cfg->addDeletable<System *>(IPET::ID_System, system);
	}

	// Add the object function
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit())
			system->addObjectFunction(
				bb->use<int>(IPET::ID_Time),
				bb->use<Var *>(IPET::ID_Var));
}

} // otawa
