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
 * Transfer flow information from the given source instruction to the given BB.
 * @param source	Source instruction.
 * @param bb		Target BB.
 * @return			True if some loop bound informatio has been found, false else.
 */
bool FlowFactLoader::transfer(Inst *source, BasicBlock *bb) {
	bool one = false;
	
	// look for MAX_ITERATION
	int count = MAX_ITERATION(source);
	if(count >= 0) {
		LOOP_COUNT(bb) = count;
		found_loop++;
		one = true;
		if(isVerbose())
			log << "\t\t\tLOOP_COUNT(" << bb << ") = " << count << io::endl;
	}
	
	// loop for CONTEXTUAL_LOOP_BOUND
	ContextualLoopBound *bound = CONTEXTUAL_LOOP_BOUND(source);
	if(bound) {
		CONTEXTUAL_LOOP_BOUND(bb) = bound;
		found_loop++;
		line_loop++;
		one = true;
		if(isVerbose())
			log << "\t\t\tCONTEXTUAL_LOOP_BOUND(" << bb << ") = " << bound << io::endl;
	}
	
	return one;
}


/**
 */
void FlowFactLoader::setup(WorkSpace *ws) {
	lines_available = ws->isProvided(SOURCE_LINE_FEATURE);
	total_loop = 0;
	found_loop = 0;
	line_loop = 0;
}


/**
 */
void FlowFactLoader::cleanup(WorkSpace *ws) {
	if(isVerbose()) {
		if(!total_loop)
			log << "\tno loop found\n";
		else {
			log << "\ttotal loop = " << total_loop << " (100%)\n";
			log << "\tfound loop = " << found_loop << " (" << ((float)found_loop * 100 / total_loop) << "%)\n";
			log << "\tline loop = " << line_loop << " (" << ((float)line_loop * 100 / total_loop) << "%)\n";
		}
	}
}


/**
 * Look for a bound for the given basic block according to a source/line 
 * found on the instruction.
 * @param inst	Instruction to look in.
 * @param bb	BB to put the bound to.
 * @return		True if the bound has been found, false else.
 */ 
bool FlowFactLoader::lookLineAt(Inst *inst, BasicBlock *bb) {
	if(!lines_available)
		return false;
	
	// get the matching line
	Option<Pair<cstring, int> > res =
		workspace()->process()->getSourceLine(inst->address());
	if(!res)
		return false;
	
	// go back to the first statement of the line
	Vector<Pair<Address, Address> > addresses;
	workspace()->process()->getAddresses((*res).fst, (*res).snd, addresses);
	ASSERT(addresses);
	Inst *line_inst = workspace()->findInstAt(addresses[0].fst);
	ASSERT(line_inst);
	return transfer(line_inst, bb);
}


/**
 */
void FlowFactLoader::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	ASSERT(ws);
	ASSERT(cfg);
	ASSERT(bb);
	if(!bb->isEnd() && Dominance::isLoopHeader(bb)) {
		total_loop++;
		
		// Look in the first instruction of the BB
		BasicBlock::InstIter iter(bb);
		ASSERT(iter);
		if(transfer(iter, bb))
			return;
		
		// Attempt to look at the start of the matching source line
		if(lookLineAt(bb->firstInst(), bb))
			return;
		
		// Look all instruction in the header
		// (in case of aggregation in front of the header)
		for(BasicBlock::InstIter inst(bb); inst; inst++)
			if(lookLineAt(inst, bb))
				
				return;
		
		// look in back edge in case of "while() ..." to "do ... while(...)" optimization
		for(BasicBlock::InIterator edge(bb); edge; edge++)
			if(Dominance::isBackEdge(edge))
				for(BasicBlock::InstIter inst(edge->source()); inst; inst++)
					if(lookLineAt(inst, bb)) 
						return;

		// warning for lacking loops
		warn(_ << "no limit for the loop at " << str(bb->address()) << ".");
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
