/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 *
 * src/prog/ipet_IPETFlowFactConstraintBuilder.h -- IPETFlowFactConstraintBuilder class implementation.
 */

#include <otawa/cfg.h>
#include <elm/io.h>
#include <otawa/ipet/FlowFactConstraintBuilder.h>
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
 * @class FlowFactConstraintBuilder
 * This processor allows using extern flow facts in an IPET system.
 * Uses the LOOP_COUNT properties provided by FlowFactLoader to build constraints
 * 
 * @par Configuration
 * @li @ref FLOW_FACTS_PATH
 * 
 * @par Required Features
 * @li @ref ipet::COLLECTED_CFG_FEATURE
 * @li @ref ipet::FLOW_FACTS_FEATURE
 * 
 * @par Provided Features
 * @li @ref ipet::FLOW_FACTS_CONSTRAINTS_FEATURE
 */


/**
 * Build a new flow fact loader.
 */
FlowFactConstraintBuilder::FlowFactConstraintBuilder(void)
:	CFGProcessor("otawa::ipet::FlowFactConstraintBuilder", Version(1, 1, 0))
{
	require(COLLECTED_CFG_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(ASSIGNED_VARS_FEATURE);
	require(FLOW_FACTS_FEATURE);
	provide(FLOW_FACTS_CONSTRAINTS_FEATURE);
}


/**
 */
void FlowFactConstraintBuilder::processCFG(WorkSpace *fw, CFG *cfg) {

	ilp::System *system = getSystem(fw, ENTRY_CFG(fw));
	
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		if (Dominance::isLoopHeader(bb) && (LOOP_COUNT(bb) != -1)) {
			cout << "AJOUTE LOOP COUNT INFO: " << bb->number() << " count " << LOOP_COUNT(bb) << "\n";
			// sum{(i,h) / h dom i} eih <= count * sum{(i, h) / not h dom x} xeih
			otawa::ilp::Constraint *cons = system->newConstraint(otawa::ilp::Constraint::LE);
			for(BasicBlock::InIterator edge(bb); edge; edge++) {
				assert(edge->source());
				otawa::ilp::Var *var = VAR(edge);
					//edge->source()->use<otawa::ilp::Var *>(VAR);
				if(Dominance::dominates(bb, edge->source()))
					cons->addLeft(1, var);
				else
					cons->addRight(LOOP_COUNT(bb), var);
			}
		}
	}
}




/**
 * This feature asserts that constraints tied to the flow fact information
 * has been added to the ILP system.
 */
Feature<FlowFactConstraintBuilder>
	FLOW_FACTS_CONSTRAINTS_FEATURE("otawa::ipet::flow_facts_constraints");

} 

} // otawa::ipet
