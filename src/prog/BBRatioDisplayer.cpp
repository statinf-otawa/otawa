/*
 *	$Id$
 *	BBRatioDisplayer processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/ipet.h>
#include <otawa/util/BBRatioDisplayer.h>

namespace otawa {

// Private identifier
otawa::Identifier<int> BBRatioDisplayer::SUM("", 0);


/**
 * @class BBRatioDisplayer
 * This class output statistics for each basic block in the WCET as a table
 * in a textual form on the standard output.
 * The following columns are available:
 * @li @c ADDRESS -- address of the BB,
 * @li @c NUM -- BB number (in the CFG),
 * @li @c SIZE -- size in bytes of the BB,
 * @li @c TIME -- time in cycles of one basic block execution,
 * @li @c COUNT -- number of times the basic block is executed in the WCET,
 * @li @c RATIO -- ratio occupied in the WCET by the cumulative execution of the BB,
 * @li @c FUNCTION -- function (if any) containing the BB.
 * 
 * In addition, a line is devoted to the same statistics for a whole function
 * and the last line sum up this column for the WCET.
 * 
 * @warning Due to overlapping of BB execution time on pipelined processors,
 * the sum of all basic blocks ratio may be a bit greater than 100%.
 * 
 * @par Required features
 * @li @ref ipet::WCET_FEATURE
 * @li @ref ipet::ASSIGNED_VARS_FEATURE
 */


/**
 * Build the processor.
 */
BBRatioDisplayer::BBRatioDisplayer(void)
: BBProcessor("BBTimeDisplayer", Version(1, 0, 0)) {
		require(ipet::WCET_FEATURE);
		require(ipet::ASSIGNED_VARS_FEATURE);
}


/**
 */
void BBRatioDisplayer::setup(WorkSpace *ws) {
	wcet = ipet::WCET(ws);
	system = ipet::SYSTEM(ws);
	out << "ADDRESS\t\tNUM\tSIZE\tTIME\tCOUNT\tRATIO\t\tFUNCTION\n";
}


/**
 */
void BBRatioDisplayer::processCFG(WorkSpace *fw, CFG *cfg) {
	BBProcessor::processCFG(fw, cfg);
	out << "TOTAL FUNCTION\t\t"
		<< SUM(cfg) << '\t'
		<< (int)system->valueOf(ipet::VAR(cfg->entry())) << '\t'
		<< (float)SUM(cfg) * 100 / wcet << "%\t\t"
		<< cfg->label() << io::endl; 		
}


/**
 */
void BBRatioDisplayer::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {
	if(bb->isEnd())
		return;
	int count = (int)system->valueOf(ipet::VAR(bb)),
		time = ipet::TIME(bb),
		total = time * count;
	SUM(cfg) = SUM(cfg) + total;
	out << bb->address() << '\t'
		<< bb->number() << '\t'
		<< bb->size() << '\t'
		<< time << '\t'
		<< count << '\t'
		<< (float)total * 100 / wcet << "%\t"
		<< cfg->label() << io::endl; 
}

} // otawa
