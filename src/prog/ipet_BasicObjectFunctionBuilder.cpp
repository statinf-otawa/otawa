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
 */


/**
 * Build a new basic object function builder.
 * @param props		Configuration properties.
 */
BasicObjectFunctionBuilder::BasicObjectFunctionBuilder(const PropList& props)
: BBProcessor("otawa::BasicObjectFunctionBuilder", Version(1, 0, 0), props) {
}


/**
 */
void BasicObjectFunctionBuilder::processBB(
	FrameWork *fw,
	CFG *cfg,
	BasicBlock *bb)
{
	if(!bb->isEntry() && !bb->isExit()) {
		System *system = getSystem(fw, ENTRY_CFG(fw));
		int time = TIME(bb);
		if(time < 0)
			throw new ProcessorException(*this, "no time on BB %lx",
				(int)bb->address());
		system->addObjectFunction(time, getVar(system, bb));
	}
}

} } // otawa::ipet
