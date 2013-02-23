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

#include <elm/system/System.h>
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
 *
 * @par Configuration
 * @li @ref otawa::BBRatioDisplay::TO_FILE
 * @li @ref otawa::BBRatioDisplay::PATH
 */

p::declare BBRatioDisplayer::reg = p::init("otawa::BBRatioDisplayer", Version(1, 0, 1))
	.require(ipet::WCET_FEATURE)
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.maker<BBRatioDisplayer>();

/**
 * Build the processor.
 */
BBRatioDisplayer::BBRatioDisplayer(AbstractRegistration& r)
: BBProcessor(r), path(""), to_file(false), stream(0), line(false) {
}


/**
 */
void BBRatioDisplayer::configure(const PropList& props) {
	BBProcessor::configure(props);
	path = PATH(props);
	to_file = TO_FILE(props);
}


/**
 */
void BBRatioDisplayer::setup(WorkSpace *ws) {

	// source available ?
	if(ws->isProvided(SOURCE_LINE_FEATURE))
		line = true;
	if(logFor(LOG_PROC))
		log << "\tsource/line information " << (line ? "" : "not ") << "available\n";

	// prepare the output
	if(!path && to_file)
		path = _ << ENTRY_CFG(ws)->label() << ".ratio";
	if(path) {
		if(logFor(LOG_PROC))
			log << "\toutputting to " << path << io::endl;
		try {
			stream = elm::system::System::createFile(path);
		}
		catch(elm::system::SystemException& exn) {
			throw ProcessorException(*this, _ << "cannot open \"" << path << "\"");
		}
		out.setStream(*stream);
	}

	// prepare the work
	wcet = ipet::WCET(ws);
	system = ipet::SYSTEM(ws);
	out << "ADDRESS\t\tNUM\tSIZE\tTIME\tCOUNT\tRATIO\t\tFUNCTION";
	if(line)
		out << "\tLINE";
	out << io::endl;
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
		<< cfg->label();
	
	// Line display
	if(line) {
		out << '\t';
		bool one = false;
		Pair<cstring, int> old = pair(cstring(""), 0);
		for(Address addr = bb->address(); addr.offset() < bb->topAddress().offset(); addr = addr + 1) {
			Option<Pair<cstring, int> > res = fw->process()->getSourceLine(addr);
			if(res) {
				if((*res).fst != old.fst) {
					old = *res;
		 			if(one)
		 				out << ", ";
		 			else
		 				one = true;
		 			out << old.fst << ':' << old.snd;
				}
				else if((*res).snd != old.snd) {
					old = *res;
					out << ',' << old.snd;
				}
			} 
		}
	}
	
	// End of line
	out << io::endl; 
}


/**
 */
void BBRatioDisplayer::cleanup(WorkSpace *ws) {
	if(stream)
		delete stream;
}


/**
 * Configure the @ref BBRatiodisplayer to perform its output to a file.
 * If no @ref BBRatioDisplayer::PATH is given, the file name is obtained
 * from the entry function name and postfixed with ".ratio".
 */
Identifier<bool> BBRatioDisplayer::TO_FILE("otawa::BBRatioDisplayer::TO_FILE", false);


/**
 * Configure the @ref BBRatioDisplayer to perform its output to the named file.
 */
Identifier<elm::system::Path> BBRatioDisplayer::PATH("otawa::BBRatioDisplayer::PATH", "");


} // otawa
