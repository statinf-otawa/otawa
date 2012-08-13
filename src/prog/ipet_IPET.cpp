/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	src/prog/ipet_IPET.cpp -- IPET class implementation.
 */

#include <elm/util/MessageException.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/manager.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/prop/info.h>

namespace otawa { namespace ipet {

using namespace ilp;

/**
 * @defgroup ipet IPET Method
 *
 * OTAWA provides many code processors supporting IPET approach:
 * @li @ref VarAssignment - assign ILP variable to each basic block and edge
 * of the graph.
 * @li @ref BasicConstraintsBuilder - build constraints generated from the
 * program CFG,
 * @li @ref BasicObjectFunctionBuilder - build the object function to maximize
 * to get the WCET,
 * @li @ref ConstraintLoader - add constraints from an external file,
 * @li @ref FlowFactLoader - load and add constraints for the flow facts
 * (currently constante loop upper bounds only),
 * @li @ref WCETComputation - perform the WCET final computation.
 *
 * Some processors are dedicated to pipeline-level analyses:
 * @li @ref TrivialBBTime - compute the execution time of a basic block by
 * applying a constant execution to each instruction,
 * @li @ref Delta - change the constraints and the object function to implement
 * the Delta method for inter-basic-block pipe-line effect support,
 * @li @ref ExeGraphBBTime - compute the execution time of basic block using
 * an execution graph.
 *
 * Some processors are dedicated to cache analyses:
 * @li @ref CATBuilder - compute the cache category of each instruction
 * (always-miss, always-hit, first-miss, first-it),
 * @li @ref CATConstraintBuilder - add constraints to use instruction access
 * categories,
 * @li @ref CCGBuilder - build the Cache Conflict Graph for the instruction
 * cache,
 * @li @ref CCGConstraintBuilder - build the constraints using the CCG to
 * take in account instruction cache accesses,
 * @li @ref CCGObjectFunction - the CCG instruction cache approach requires
 * to use a special object function in order to get the WCET,
 * @li @ref TrivialDataCacheManager - consider each memory access as a miss.
 *
 * These processors interacts using the following properties:
 * @li @ref TIME - execution time of a program part (usually a basic block),
 * @li @ref VAR - ILP variable associated with a program component (usually
 * a basic block or an edge),
 * @li @ref SYSTEM - ILP system associated with a root CFG,
 * @li @ref WCET - WCET of a root CFG,
 * @li @ref EXPLICIT - used in configuration of code processor, cause the
 * generation of explicit variable names instead of numeric ones (default to false),
 * @li @ref RECURSIVE - cause the processors to work recursively (default to false).
 */


/**
 * This identifier is used for storing the time of execution in cycles (int)
 * of the program area it applies to.
 * @ingroup ipet
 */
Identifier<ot::time> TIME("otawa::ipet::TIME", -1,
	idLabel("time"),
	idDesc("execution time (in cycles)"),
	0);


/**
 * This identifier is used for storing in basic blocks and edges the variables
 * (otawa::ilp::Var *) used in ILP resolution.
 * @ingroup ipet
 */
Identifier<ilp::Var *> VAR("otawa::ipet::VAR", 0,
	idLabel("variable"),
	idDesc("variable in ILP system for IPET resolution"),
	0);


/**
 * Identifier of annotation used for storing for storing the WCET value (int)
 * in the CFG of the computed function.
 * @ingroup ipet
 */
Identifier<ot::time> WCET("otawa::ipet::WCET", -1,
	idLabel("WCET"),
	idDesc("WCET (in cycles)"),
	0);


/**
 * Identifier of a boolean property requiring that explicit names must be used.
 * The generation of explicit names for variables may be time-consuming and
 * must only be activated for debugging purposes.
 * @ingroup ipet
 */
Identifier<bool> EXPLICIT("otawa::ipet::EXPLICIT", false);


/**
 * This property is used to store time delta on edges. This time may be used
 * to improve accuracy of the IPET system with a time modifier based on
 * edges (use the @ref TimeDeltaObjectFunctionModifier to add time deltas
 * to the object function).
 * @ingroup ipet
 */
Identifier<ot::time> TIME_DELTA("otawa::ipet::TIME_DELTA", 0,
	idLabel("time delta"),
	idDesc("time fix for an edge traversal (in cycles)"),
	0);


/**
 * This property is put on basic blocks and edge to record the execution count
 * of these object on the WCET path.
 *
 * @par Feature
 * @li @ref ipet::WCET_COUNT_RECORDED_FEATURE
 *
 * @par Hooks
 * @li @ref otawa::BasicBlock
 * @li @ref otawa::Edge
 * @ingroup ipet
 */
Identifier<int> COUNT("otawa::ipet::COUNT", -1,
	idLabel("execution count"),
	idDesc("execution count in WCET"),
	0);


/**
 * Get the system tied with the given CFG. If none exists, create ones.
 * @param fw	Current workspace.
 * @param cfg	Current CFG.
 * @preturn		CFG ILP system.
 */
ilp::System *getSystem(WorkSpace *fw, CFG *cfg) {
	return SYSTEM(fw);
}


/**
 * This feature ensures that effects of the inter-block have been modelized
 * in the current ILP system. Currently, there is no default processor to get
 * it cause the heterogeneity of solutions to this problem.
 *
 * @par Implementing Processors
 * @ref @li Delta
 * @ingroup ipet
 */
Feature<NoProcessor>
	INTERBLOCK_SUPPORT_FEATURE("otawa::ipet::INTERBLOCK_SUPPORT_FEATURE");

} } // otawa::ipet
