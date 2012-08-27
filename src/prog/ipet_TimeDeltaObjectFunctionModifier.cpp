/*
 *	$Id$
 *	Copyright (c) 2006-07, IRIT UPS.
 */


#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/ipet/TimeDeltaObjectFunctionModifier.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/proc/ProcessorException.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

/**
 * @class TimeDeltaObjectFunctionModifier
 * This processor is used for modify the object function
 * to maximize for resolving the IPET system, that is,
 * @p For each edge where a @ref TIME_DELTA is found, it adds:
 * @p	tdelta * ei,j
 * @p where
 * <dl>
 * 	<dt>tdelta</dt><dd>Delta time found on edge (i, j).</dd>
 *  <dt>ei,j</dt><dd>Count of execution of edge (i, j).</dd>
 * </dl>
 *
 * @par Provided Feature
 * @li @ref EDGE_TIME_FEATURE
 *
 * @par Required Feature
 * @li @ref ILP_SYSTEM_FEATURE
 */

p::declare TimeDeltaObjectFunctionModifier::reg = p::init("otawa::ipet::TimeDeltaObjectFunctionModifier", Version(1, 0, 1))
	.provide(EDGE_TIME_FEATURE)
	.require(ILP_SYSTEM_FEATURE)
	.require(OBJECT_FUNCTION_FEATURE)
	.maker<TimeDeltaObjectFunctionModifier>();


/**
 * Build a new basic object function builder.
 * @param props		Configuration properties.
 */
TimeDeltaObjectFunctionModifier::TimeDeltaObjectFunctionModifier(p::declare& r): BBProcessor(r) {
}


/**
 */
void TimeDeltaObjectFunctionModifier::processBB(
	WorkSpace *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	if(!bb->isEntry()) {
		System *system = SYSTEM(fw);
		for(BasicBlock::InIterator edge(bb); edge; edge++) {
			int time = TIME_DELTA(edge);
			if(time)
				system->addObjectFunction(time, VAR(edge));
		}
	}
}

/**
 * This feature ensurers that the @ref TIME_DELTA property linked to the CFG edges are
 * used in the maximized object function of the IPET ILP system.
 */
Feature<TimeDeltaObjectFunctionModifier> EDGE_TIME_FEATURE("otawa::ipet::EDGE_TIME_FEATURE");

} } // otawa::ipet
