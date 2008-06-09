/*
 *	$Id$
 *	ipet::FlowFactLoader class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/util/Dominance.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/flowfact/ContextualLoopBound.h>
#include <otawa/flowfact/features.h>

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
		bool one = false;
		
		// look for MAX_ITERATION
		int count = MAX_ITERATION(iter);
		if(count >= 0) {
			LOOP_COUNT(bb) = count;
			one = true;
			if(isVerbose())
				log << "\t\t\tLOOP_COUNT(" << bb << ") = " << count << io::endl;
		}
		
		// loop for CONTEXTUAL_LOOP_BOUND
		ContextualLoopBound *bound = CONTEXTUAL_LOOP_BOUND(iter);
		if(bound) {
			CONTEXTUAL_LOOP_BOUND(bb) = bound;
			one = true;
			if(isVerbose())
				log << "\t\t\tCONTEXTUAL_LOOP_BOUND(" << bb << ") = " << bound << io::endl;
		}

		// warning for lacking loops
		if(!one)
			warn(_ << "no limit for the loop at " << bb->address() << ".");
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
