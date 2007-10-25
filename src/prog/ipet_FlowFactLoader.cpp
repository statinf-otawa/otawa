/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 *
 * src/prog/ipet_IPETFlowFactLoader.h -- IPETFlowFactLoader class implementation.
 */

#include <otawa/cfg.h>
#include <elm/io.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/util/Dominance.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/util/FlowFactLoader.h>

namespace otawa { namespace ipet {

/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in an IPET system.
 * 
 * @par Configuration
 * @li @ref FLOW_FACTS_PATH
 * 
 * @par Required Features
 * @li @ref ipet::LOOP_HEADERS_FEATURE
 * @li @ref otawa::FLOW_fACTS_FEATURE
 * 
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_FEATURE
 */


/**
 * Build a new flow fact loader.
 */
FlowFactLoader::FlowFactLoader(void)
:	BBProcessor("otawa::ipet::FlowFactLoader", Version(1, 1, 0)) {
	require(LOOP_HEADERS_FEATURE);
	require(otawa::FLOW_FACTS_FEATURE);
	provide(otawa::ipet::FLOW_FACTS_FEATURE);
}


/**
 */
void FlowFactLoader::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	ASSERT(ws);
	ASSERT(cfg);
	ASSERT(bb);
	if(!bb->isEnd() && Dominance::isLoopHeader(bb)) {
		BasicBlock::InstIterator iter(bb);
		ASSERT(iter);
		int count = MAX_ITERATION(iter);
		if(count < 0)
			warn(_ << "no limit for the loop at " << bb->address() << ".");
		else
			LOOP_COUNT(bb) = count;
	}
}


/**
 * This feature ensures that flow facts information (at less the loop bounds)
 * has been put on the CFG of the current task.
 * 
 * @par Properties
 * @li @ref ipet::LOOP_COUNT
 */
Feature<FlowFactLoader> FLOW_FACTS_FEATURE("otawa::ipet::flow_facts");


} } // otawa::ipet
