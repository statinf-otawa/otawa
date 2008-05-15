/*
 *	$Id$
 *	exegraph module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <otawa/exegraph/ExeGraphBBTime.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;

//#define ACCURATE_STATS
#if defined(NDEBUG) || !defined(ACCURATE_STATS)
#	define STAT(c)
#else
#	define STAT(c) c
#endif

namespace otawa { 

/**
 * @class ExecutionGraphBBTime
 * @brief This basic block processor computes the basic block execution time using
 * an execution graph (ExeGraph)
 */


/**
 * @fn ExecutionGraphBBTime::ExeGraphBBTime
 * Builds the ExecutionGraphBBTime processor.
 * @param props	Configuration properties possibly including @ref PROC and
 * @ref LOG.
 */ 

/**
 * @fn ExecutionGraphBBTime::processWorkSpace(FrameWork *fw)
 * Builds the processor model if it does not exist yet.
 * @param	fw	The framework.
 */

} 

/**
 * @fn ExecutionGraphBBTime::processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb)
 * Computes the execution time of a basic block. This function is empty
 * and should be overloaded in derived execution graph classes.
 * @param	fw	The framework.
 * @param	cfg	The Control Flow Graph of the program under analysis.
 * @param	bb	The basic block to be analyzed.
 * */

 
