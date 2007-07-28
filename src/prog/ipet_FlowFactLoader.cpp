/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 *
 * src/prog/ipet_IPETFlowFactLoader.h -- IPETFlowFactLoader class implementation.
 */

#include <otawa/cfg.h>
#include <elm/io.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/IPET.h>
#include <otawa/util/Dominance.h>
#include <otawa/ilp.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/util/Dominance.h>
#include <otawa/ipet/VarAssignment.h>

namespace otawa { namespace ipet {

/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in an IPET system.
 * 
 * @par Configuration
 * @li @ref FLOW_FACTS_PATH
 * 
 * @par Required Features
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref ipet::COLLECTED_CFG_FEATURE
 * @li @ref ipet::LOOP_HEADERS_FEATURE
 * 
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_FEATURE
 */


/**
 * Build a new flow fact loader.
 */
FlowFactLoader::FlowFactLoader(void)
:	Processor("otawa::ipet::FlowFactLoader", Version(1, 1, 0)),
	path(""),
	verbose(false)
{
	require(ILP_SYSTEM_FEATURE);
	require(COLLECTED_CFG_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	provide(FLOW_FACTS_FEATURE);
}


/**
 */
void FlowFactLoader::onError(const char *fmt, ...) {
	assert(fmt);
	VARARG_BEGIN(args, fmt)
		StringBuffer buffer;
		buffer.format(fmt, args);
		throw ProcessorException(*this, buffer.toString());
	VARARG_END
}


/**
 */
void FlowFactLoader::onWarning(const char *fmt, ...) {
	assert(fmt);
	VARARG_BEGIN(args, fmt)
	StringBuffer buffer;
	buffer.format(fmt, args);
	warn(buffer.toString());
	VARARG_END
}


/**
 */
void FlowFactLoader::onLoop(address_t addr, int count) {
	// !!TODO!! Should be replaced by more efficient solution
	// using maybe an has table for storing loop header / loop count.
	assert(system);
	assert(count >= 0);
	//cout << "LOOP " << count << " times at " << addr << "\n";

	// Process basic blocks
	bool found = false;
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++) {

		// Look BB in the CFG
		for(CFG::BBIterator bb(cfg); bb; bb++)
			if(bb->address() == addr && Dominance::isLoopHeader(bb)) {
			
				LOOP_COUNT(bb) = count;

				found = true;
			}
	}
	
	// Nothing found, seems too bad
	if(!found)
		out << "WARNING: loop " << addr << " not found.\n";
}


/**
 */
void FlowFactLoader::processWorkSpace(WorkSpace *fw) {
	assert(fw);
	cfgs = INVOLVED_CFGS(fw);
	assert(cfgs);
	system = SYSTEM(fw);
	run(fw, path, verbose);
}


/**
 */
void FlowFactLoader::configure(const PropList& props) {
	Processor::configure(props);
	path = FLOW_FACTS_PATH(props);
	verbose = VERBOSE(props);
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
