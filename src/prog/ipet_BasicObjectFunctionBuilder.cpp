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
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/VarAssignment.h>
#include <otawa/ipet/TrivialBBTime.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

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
 * @par Required Features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 * @li @ref ipet::BB_TIME_FEATURE
 * 
 * @par Provided Features
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 */


/**
 * Build a new basic object function builder.
 */
BasicObjectFunctionBuilder::BasicObjectFunctionBuilder(void)
: BBProcessor("otawa::ipet::BasicObjectFunctionBuilder", Version(1, 0, 0)) {
	require(ASSIGNED_VARS_FEATURE);
	require(BB_TIME_FEATURE);
	provide(OBJECT_FUNCTION_FEATURE);
}


/**
 */
void BasicObjectFunctionBuilder::processBB(
	WorkSpace *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	if(!bb->isEntry() && !bb->isExit()) {
		System *system = getSystem(fw, ENTRY_CFG(fw));
		int time = TIME(bb);
		if(time < 0)
			throw ProcessorException(*this, "no time on BB %lx (%d of %s)",
				(int)bb->address(), bb->number(), &cfg->label());
		system->addObjectFunction(time, getVar(system, bb));
	}
}

/**
 * This feature ensures that the object function of the ILP system to solve
 * has been built.
 */
Feature<BasicObjectFunctionBuilder>
	OBJECT_FUNCTION_FEATURE("ipet::object_function");


} } // otawa::ipet
