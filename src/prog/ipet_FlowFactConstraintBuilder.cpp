/*
 *	$Id$
 *	ipet::FlowFactLoader class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
 * @li @ref ipet::ILP_SYSTEM_FEATURE
 * @li @ref ipet::COLLECTED_CFG_FEATURE
 * @li @ref ipet::FLOW_FACTS_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
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

	ilp::System *system = SYSTEM(fw);
	
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		if (Dominance::isLoopHeader(bb)) {
			if(LOOP_COUNT(bb) == -1)
			 	warn(_ << "no loop count for header at " << bb->address());
			else {
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
}




/**
 * This feature asserts that constraints tied to the flow fact information
 * has been added to the ILP system.
 */
Feature<FlowFactConstraintBuilder>
	FLOW_FACTS_CONSTRAINTS_FEATURE("otawa::ipet::flow_facts_constraints");

} 

} // otawa::ipet
