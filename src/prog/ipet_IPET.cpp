/*
 *	IPET module implementation
 *
 *	This file is part of OTAWA.
 *	Copyright (c) 2005-17, IRIT UPS <casse@irit.fr>
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <elm/util/MessageException.h>

#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/cat2/features.h>
#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/manager.h>
#include <otawa/proc/DynFeature.h>
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
p::feature INTERBLOCK_SUPPORT_FEATURE("otawa::ipet::INTERBLOCK_SUPPORT_FEATURE", new Maker<NoProcessor>());


/**
 * This feature ensurers that the instruction cache has been modeled
 * in the IPET approach.
 *
 * Default Analysis:
 * @li @ref otawa::trivial::AllMissICacheTime
 */
p::feature INST_CACHE_SUPPORT_FEATURE("otawa::ipet::INST_CACHE_SUPPORT_FEATURE", new DelayedMaker("otawa::trivial::AllMissICacheTime"));


/**
 * This feature ensures that the first-level data cache has been taken in
 * account in the basic block timing.
 *
 * Default Analysis:
 * @li @ref otawa::trivial::NoDCacheTime
 */
p::feature DATA_CACHE_SUPPORT_FEATURE("otawa::ipet::DATA_CACHE_SUPPORT_FEATURE", new DelayedMaker("otawa::trivial::NoDCacheTime"));


/**
 * Provide support for cache hierarchy. Basically, explore the cache hierarchy
 * and either select the matching analyzes, or raise an error.
 */
class CacheSupport: public Processor {
public:
	static p::declare reg;
	CacheSupport(p::declare& _reg = reg): Processor(_reg) { }

protected:
	virtual void prepare(WorkSpace *ws) {
		const hard::CacheConfiguration& conf = *hard::CACHE_CONFIGURATION_FEATURE.get(ws);

		// no cache needed?
		if(!conf.instCache() && !conf.dataCache()) {
			if(logFor(LOG_PROC))
				log << "\tINFO: no cache.";
			return;
		}

		// unified?
		if(conf.isUnified())
			throw ProcessorException(*this, "unified L1 cache unsupported");

		// process instruction cache
		if(conf.instCache()) {
			if(conf.instCache()->replacementPolicy() != hard::Cache::LRU)
				throw ProcessorException(*this, _ << "instruction cache L1 unsupported");
			require(ICACHE_ACS_FEATURE);
			require(ICACHE_ACS_MAY_FEATURE);
			require(ICACHE_CATEGORY2_FEATURE);
			require(ICACHE_ONLY_CONSTRAINT2_FEATURE);
		}

		// process data cache
		if(conf.dataCache()) {
			if(conf.dataCache()->replacementPolicy() != hard::Cache::LRU)
				throw ProcessorException(*this, _ << "replacement policy of data cache L1 unsupported");
			// TODO		For now, supports only write-through data cache (write-back to be added later)
			if(conf.dataCache()->writePolicy() != hard::Cache::WRITE_THROUGH)
				throw ProcessorException(*this, _ << "write policy of data cache L1 unsupported");
			requireDyn("otawa::dcache::MUST_ACS_FEATURE");
			requireDyn("otawa::dcache::PERS_ACS_FEATURE");
			requireDyn("otawa::dcache::MAY_ACS_FEATURE");
			requireDyn("otawa::dcache::CONSTRAINTS_FEATURE");
		}
	}

private:
	void requireDyn(cstring name) {
		DynFeature f(name);
		require(f);
	}
};

p::declare CacheSupport::reg = p::init("otawa::ipet::CacheSupport", Version(1, 0, 0))
	.maker<CacheSupport>()
	.require(hard::CACHE_CONFIGURATION_FEATURE)
	.provide(CACHE_SUPPORT_FEATURE);


/**
 * This feature ensures that analysis for the cache configuration has been performed.
 */
p::feature CACHE_SUPPORT_FEATURE("otawa::ipet::CACHE_SUPPORT_FEATURE", new Maker<CacheSupport>());

/**
 * This feature ensures that the execution time of each basic block has been
 * computed.
 *
 * @par Properties
 * @li otawa::ipet::TIME
 * @li otawa::ipet::DELTA_TIME
 */
p::feature BB_TIME_FEATURE("otawa::ipet::BB_TIME_FEATURE", new DelayedMaker("otawa::trivial::BlockTime"));

/**
 * This features ensure that pipeline time has been produced.
 * This time model is more powerful than the original @ref BB_TIME_FEATURE
 * as it allows to associate several times (with several counter) with the BBs
 * and the edges.
 * 
 * **Default processor:** none.
 * 
 * **Properties:** @ref PIPELINE_TIME
 * 
 * @ingroup ipet
 * 
 */
p::feature PIPELINE_TIME_FEATURE("otawa::ipet::PIPELINE_TIME_FEATURE");


/**
 * Property recording one time for a BB or an edge and its frequency.
 * Its arguments ia a pair (t, x) where time t is the time and x is the ILP
 * variable accounting for the frequency.
 * 
 * **Features:** @ref PIPELINE_TIME_FEATURE
 * **Hooks:** @ref BasicBlock, @ref Edge . 
 * @ingroup ipet
 */
p::id<pipeline_time_t> PIPELINE_TIME("otawa::ipet::PIPELINE_TIME");

} } // otawa::ipet
