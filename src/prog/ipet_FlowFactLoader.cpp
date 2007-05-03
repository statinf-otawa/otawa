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
 * @li @ref ipet::COLLECTED_CFG_FEATURE
 * 
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_FEATURE
 * @li @ref ipet::FLOW_FACTS_CONSTRAINTS_FEATURE
 */


/**
 * Build a new flow fact loader.
 */
FlowFactLoader::FlowFactLoader(void)
:	Processor("otawa::ipet::FlowFactLoader", Version(1, 1, 0)),
	path(""),
	verbose(false)
{
	require(COLLECTED_CFG_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(ASSIGNED_VARS_FEATURE);
	provide(FLOW_FACTS_FEATURE);
	provide(FLOW_FACTS_CONSTRAINTS_FEATURE);
}


/**
 */
void FlowFactLoader::onError(const char *fmt, ...) {
	assert(fmt);
	VARARG_BEGIN(args, fmt)
		throw ProcessorException(*this, fmt, args);
	VARARG_END
}


/**
 */
void FlowFactLoader::onWarning(const char *fmt, ...) {
	assert(fmt);
	VARARG_BEGIN(args, fmt)
	warn(fmt, args);
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
			if(bb->address() == addr /*&& Dominance::isLoopHeader(bb)*/) {
			
				// Build the constraint
				// sum{(i,h) / h dom i} eih <= count * sum{(i, h) / not h dom x} xeih 
				LOOP_COUNT(bb) = count;
				otawa::ilp::Constraint *cons =
					system->newConstraint(otawa::ilp::Constraint::LE);
				for(BasicBlock::InIterator edge(bb); edge; edge++) {
					assert(edge->source());
					otawa::ilp::Var *var = VAR(edge);
						//edge->source()->use<otawa::ilp::Var *>(VAR);
					if(Dominance::dominates(bb, edge->source()))
						cons->addLeft(1, var);
					else
						cons->addRight(count, var);
				}
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
	system = getSystem(fw, ENTRY_CFG(fw));
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


/**
 * This feature asserts that constraints tied to the flow fact information
 * has been added to the ILP system.
 */
Feature<FlowFactLoader>
	FLOW_FACTS_CONSTRAINTS_FEATURE("otawa::ipet::flow_facts_constraints");

} } // otawa::ipet
