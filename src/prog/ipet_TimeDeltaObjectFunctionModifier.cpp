/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 */


#include <otawa/ipet/IPET.h>
#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/ipet/TimeDeltaObjectFunctionModifier.h>
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
 */


/**
 * Build a new basic object function builder.
 * @param props		Configuration properties.
 */
TimeDeltaObjectFunctionModifier::TimeDeltaObjectFunctionModifier(
	const PropList& props)
: 	BBProcessor(
		"otawa::TimeDeltaObjectFunctionModifier",
		Version(1, 0, 0),
		props)
{
}


/**
 */
void TimeDeltaObjectFunctionModifier::processBB(
	FrameWork *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	if(!bb->isEntry() && !bb->isExit()) {
		System *system = getSystem(fw, ENTRY_CFG(fw));
		for(BasicBlock::InIterator edge(bb); edge; edge++) {
			int time = TIME_DELTA(edge);
			if(time)
				system->addObjectFunction(time, getVar(system, edge));
		}
	}
}

} } // otawa::ipet
