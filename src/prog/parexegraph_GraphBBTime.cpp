/*
 *	GraphBBTime and BasicGraphBBTime implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-12 IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#include <otawa/parexegraph/GraphBBTime.h>
#include <otawa/parexegraph/BasicGraphBBTime.h>
#include <elm/io/OutFileStream.h>

using namespace otawa;
using namespace otawa::hard;
using namespace elm;
using namespace elm::genstruct;
using namespace otawa::graph;
using namespace otawa::ipet;

namespace otawa { 

Identifier<String> GRAPHS_OUTPUT_DIRECTORY("otawa::GRAPHS_OUTPUT_DIRECTORY","");
    
    //Feature<GraphBBTime> ICACHE_ACCURATE_PENALTIES_FEATURE("otawa::ICACHE_ACCURATE_PENALTIES_FEATURE");


  
/**
 * @class GraphBBTime
 *
 * This basic block processor computes the basic block execution time using
 * the execution graph method described in the following papers:
 *
 * <p>
 * Christine Rochange, Pascal Sainrat. <i>A Context-Parameterized Model
 * for Static Analysis of Execution Times.</i>
 * Transactions on High-Performance Embedded Architecture and Compilation, Springer,
 * Vol. 2 N. 3, p. 109-128, 2009.
 * </p>
 *
 * @par Provided Features
 * @li @ref ipet::BB_TIME_FEATURE
 *
 * @par Required Features
 * @li @ref hard::PROCESSOR_FEATURE,
 * @li @ref hard::MEMORY_FEATURE,
 *
 * @par Configuration
 * @li @ref GRAPHS_OUTPUT_DIRECTORY -- if given, gets the path of an existing directory
 * where the execution graphs will be stored in (mainly used for debugging purpose).
 */


/**
 * This property is used to pass the microprocessor description to the
 * code processor. As default, it is extracted from the system description.
 */


/**
 * This property gives an output stream used to output log messages about
 * the execution of the algorithm.
 */
//GenericIdentifier<elm::io::Output *>  ExecutionGraphBBTime::LOG_OUTPUT("otawa.ExecutionGraphBBTime.log", &cerr);


/**
 * Build the ExecutionGraphBBTime processor.
 * @param props	Configuration properties possibly including @ref PROC and
 * @ref LOG.
 */ 



/**
 * Initialize statistics collection for the given basic block.
 * @param bb	Basic block to use.
 */


/**
 * Record statistics for the given sequence.
 * @param sequence	Sequence to use.
 * @param cost		Cost of the sequence.
 */


/**
 * Record the given node in the prefix tree.
 * @param parent	Parent node.
 * @param bb		Current BB.
 * @param cost		Cost of the current prefix.
 * @return			Node matching the given BB.
 */


/**
 * Record statistics for the given node.
 * @param node	Node to record stats for.
 * @param cost	Cost of the current prefix.
 */




/**
 * Call to end the statistics of the current basic block.
 */


/**
 * Record the time delta on the input edges of the current block.
 * @param insts	Prologue.
 * @param cost	Cost of the block with the current prologue.
 * @param bb	Current basic block.
 */



/**
 * If the statistics are activated, this property is returned in statistics
 * property list to store @ref ExecutionGraphBBTime statistics.
 */
//GenericIdentifier<Vector <stat_t> *>
//	EXEGRAPH_PREFIX_STATS("exegraph_prefix_stats", 0, OTAWA_NS);
//
//
///**
// * If set to the true, ExeGraph will also put @ref TIME_DELTA properties on
// * edges. Using the @ref TimeDetaObjectFunctionModifier, it allow to improve
// * the accuracy of the computed WCET.
// */
//GenericIdentifier<bool> EXEGRAPH_DELTA("exegraph_delta", false, OTAWA_NS);
//
//
///**
// * Set to true in the configuration @ref ExecutionGraphBBTime configure, this
// * processor will generate extra contraints and objects functions factor
// * taking in account the difference of execution time according prologues
// * of evaluated blocks.
// */
//GenericIdentifier<bool> EXEGRAPH_CONTEXT("exegraph_context", false, OTAWA_NS);
//

/**
 * Compute extra context to handle context.
 * @param cost	Cost of the BB.
 * @param stat	Statistics node.
 * @param ctx	Context previous node.
 */


/**
 * @class BasicGraphBBTime
 * Basic implementation of a parametric exegraph based a GraphBBTime
 * with a ParExeGraph. Supports the basic pipeline description of OTAWA
 * with L1 instruction cache with persistence.
 */


/**
 * @fn void GraphBBTime<G>::outputGraph(G* graph, int bb_number, int context_index, int case_index, const string& info);
 * Output the given graph in the directory configured by @ref GRAPHS_OUTPUT_DIRECTORY. The name of the file is formed
 * by the concatenation of:
 *
 * <pre>
 * "b" bb_number "-ctxt" context_index "-case" case_index ".dot";
 * 	</pre>
 *
 * @param graph				Graph to output.
 * @param bb_number			Basic number.
 * @param context_index		Index of the context.
 * @param case_index		Index of case.
 * @param info				Information to display in the graph.
 */


/**
 */
p::declare BasicGraphBBTime::reg =
	p::init("otawa::BasicGraphBBTime", Version(1, 0, 0))
	.base(GraphBBTime<ParExeGraph>::reg)
	.maker<BasicGraphBBTime>();


/**
 * Constructor.
 */
BasicGraphBBTime::BasicGraphBBTime(AbstractRegistration& r): GraphBBTime<ParExeGraph>(r) {
}

}  // otawa
