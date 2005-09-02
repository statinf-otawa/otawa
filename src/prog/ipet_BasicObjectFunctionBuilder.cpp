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
: CFGProcessor("otawa::BasicObjectFunctionBuilder", Version(1, 0, 0), props) {
}


/**
 */
void BasicObjectFunctionBuilder::processCFG(FrameWork *fw, CFG *cfg) {
	System *system = IPET::getSystem(fw, cfg);

	// Add the object function
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(!bb->isEntry() && !bb->isExit()) {
			Option<int> time = bb->get<int>(IPET::ID_Time);
			if(time < 0)
				throw new ProcessorException(*this, "no time on BB %lx",
					bb->address());
			system->addObjectFunction(*time, IPET::getVar(system, bb));
		}
}

} } // otawa::ipet
