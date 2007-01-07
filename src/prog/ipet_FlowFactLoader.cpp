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
 */


/**
 * Build a new flow fact loader.
 * @param props		Configuration properties.
 * 
 * @par Required Features
 * @li @ref ipet::COLLECTED_CFG_FEATURE
 * 
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_FEATURE
 * @li @ref ipet::FLOW_FACTS_CONSTRAINTS_FEATURE
 */
FlowFactLoader::FlowFactLoader(void)
: Processor("otawa::ipet::FlowFactLoader", Version(1, 0, 0)) {
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
				//cout << "Added to " << *bb << "\n";
				otawa::ilp::Constraint *cons =
					system->newConstraint(otawa::ilp::Constraint::LE);
				cons->addLeft(1, bb->use<otawa::ilp::Var *>(VAR));
				for(BasicBlock::InIterator edge(bb); edge; edge++) {
					assert(edge->source());
					otawa::ilp::Var *var =
						edge->source()->use<otawa::ilp::Var *>(VAR);
					if(!Dominance::dominates(bb, edge->source()))
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
void FlowFactLoader::processFrameWork(FrameWork *fw) {
	assert(fw);
	cfgs = INVOLVED_CFGS(fw);
	assert(cfgs);
	system = getSystem(fw, ENTRY_CFG(fw));
	run(fw);
}


/**
 * This feature ensures that flow facts information (at less the loop bounds)
 * has been put on the CFG of the current task.
 * 
 * @par Properties
 * @li !!TODO!!
 */
Feature<FlowFactLoader> FLOW_FACTS_FEATURE("otawa::ipet::flow_facts");


/**
 * This feature asserts that constraints tied to the flow fact information
 * has been added to the ILP system.
 */
Feature<FlowFactLoader>
	FLOW_FACTS_CONSTRAINTS_FEATURE("otawa::ipet::flow_facts_constraints");

} } // otawa::ipet
