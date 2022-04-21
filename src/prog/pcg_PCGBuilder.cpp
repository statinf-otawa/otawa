/*
 *	PCGBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <elm/assert.h>
#include <elm/PreIterator.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/Edge.h>
#include <otawa/graph/LoopIdentifier.h>
#include <otawa/pcg/PCG.h>
#include <otawa/pcg/PCGBuilder.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;
using namespace otawa;

//#define DO_TRACE
#if defined(NDEBUG) && !defined(DO_TRACE)
#	define TRACE(c)
#else
#	define TRACE(c) cout << c
#endif

namespace otawa {

/**
 * @class PCGBuilder
 * Default implementation providing PCG_FEATURE.
 *
 * @par Provided features
 *	* PCG_FEATURE
 *
 * @par Required features
 *	* COLLECTED_CFG_FEATURE
 *
 * @ingroup pcg
 */

///
p::declare PCGBuilder::reg = p::init("otawa::PCGBuilder", Version(2, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.provide(PCG_FEATURE)
	.extend<Processor>()
	.make<PCGBuilder>();


/**
 */
PCGBuilder::PCGBuilder(p::declare& r): Processor(r), _pcg(0) {
}


/**
 */
void PCGBuilder::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = otawa::INVOLVED_CFGS(ws);

	// prepare the builder
	CFGCollection::Iter cfg(coll);
	_pcg = new PCG();
	graph::GenDiGraphBuilder<PCGBlock, PCGEdge> builder(_pcg);

	// make the entry node
	_pcg->_entry = new PCGBlock(*cfg);
	builder.add(_pcg->_entry);
	map.put(*cfg, _pcg->_entry);

	// build a block for each CFG
	for(cfg++; cfg(); cfg++) {
		PCGBlock *b = new PCGBlock(*cfg);
		map.put(*cfg, b);
		builder.add(b);
	}

	// build the links
	for(CFGCollection::Iter cfg(coll); cfg(); cfg++)
		for(auto call: cfg->callers())
			builder.add(map.get(call->caller()), map.get(*cfg), new PCGEdge(call));

	// install all
	builder.build();
	PROGRAM_CALL_GRAPH(ws) = _pcg;
	map.clear();
}


/**
 */
void PCGBuilder::destroy(WorkSpace *ws) {
	if(_pcg)
		delete _pcg;
}


/**
 * Analysis identifying entry of recursive function call nests.
 *
 *
 */
class ResursivityAnalysis: public Processor {
public:
	static p::declare reg;
	ResursivityAnalysis(): Processor(reg) { }

protected:

	void destroy(WorkSpace *ws) {
		PCG *pcg = PROGRAM_CALL_GRAPH(ws);
		ASSERT(pcg);
		for(auto v: *pcg)
			if(RECURSE_HEAD(v)) {
				RECURSE_HEAD(v).remove();
				for(auto e: v->inEdges())
					if(RECURSE_BACK(e).exists())
						RECURSE_BACK(e).remove();
					else
						RECURSE_ENTRY(e).remove();
			}
	}

	void processWorkSpace(WorkSpace *ws) override {
		PCG *pcg = PROGRAM_CALL_GRAPH(ws);
		ASSERT(pcg);

		graph::LoopIdentifier lid(*pcg, pcg->entry());
		for(auto v: *pcg) {

			// true header
			if(lid.isHeader(v)) {
				RECURSE_HEAD(v) = v;
				RECURSE_HEAD(v->cfg()) = v;
				if(logFor(LOG_CFG))
					log << "\t" << v << " is recursivity nest header!\n";
				for(auto e: v->inEdges())
					if(lid.loopOf(e->source()) == v) {
						RECURSE_BACK(e) = true;
						RECURSE_BACK(e->call()) = true;
						if(logFor(LOG_CFG))
							log << "\t\t" << e << " is back (" << e->call() << ")\n";
					}
					else {
						RECURSE_ENTRY(e) = true;
						RECURSE_ENTRY(e->call()) = true;
						if(logFor(LOG_BB))
							log << "\t\t" << e << " is entry\n";
					}
			}

			// re-entry
			else if(lid.isReentry(v)) {
				PCGBlock *h = static_cast<PCGBlock *>(lid.loopOf(v));
				RECURSE_HEAD(v) = h;
				if(logFor(LOG_BB))
					log << "\t" << v << " is recursivity nest re-entry with "
						<< h << " as theader!\n";
				for(auto e: v->inEdges())
					if(lid.loopOf(e->source()) == v) {
						RECURSE_BACK(e) = true;
						RECURSE_BACK(e->call()) = true;
						if(logFor(LOG_CFG))
							log << "\t\t" << e << " is back\n";
					}
					else {
						RECURSE_ENTRY(e) = true;
						RECURSE_ENTRY(e->call()) = true;
						if(logFor(LOG_BB))
							log << "\t\t" << e << " is entry\n";
					}
			}
		}
	}

};

///
p::declare ResursivityAnalysis::reg = p::init("otawa::ResursivityAnalysis", Version(1, 0, 0))
	.require(PCG_FEATURE)
	.provide(RECURSIVITY_ANALYSIS)
	.make<ResursivityAnalysis>();


/**
 * This feature ensure that a PCG is provided.
 *
 * @par Properties
 * @li @ref PROGRAM_CALL_GRAPH
 *
 * @par Default implementation
 *	* otawa::PCGBuilder
 *
 * @ingroup pcg
 */
p::feature PCG_FEATURE("otawa::PCG_FEATURE", p::make<PCGBuilder>());


/**
 * Property providing the PCG (Program Call Graph).
 *
 * Provided by:
 * @li @ref PCG_FEATURE
 *
 * Hooked to:
 * @li @ref WorkSpace
 *
 * @ingroup pcg
 */
p::id<PCG *> PROGRAM_CALL_GRAPH("otawa::PROGRAM_CALL_GRAPH", 0);


/**
 * This features ensures that recursive function calls has been identified
 * in the PCG. The PCG vertices entry points, and the corresponding CFG,
 * of recursive function nests are marked with @ref RECURSIVE.
 * In case of recursive nest with several entry points, one is randomly
 * selected as the main entry point and the @ref RECURSIVE property returns
 * this one.
 *
 * @par Properties
 *	* otawa::RECURSE_HEAD
 *	* otawa::RECURSE_ENTRY
 *	* otawa::RECURSE_BACK
 *
 * @par Implementations
 *	* ResursivityAnalysis (default)
 *
 * @ingroup pcg
 */
p::feature RECURSIVITY_ANALYSIS("otawa::RECURSIVITY_ANALYSIS", p::make<ResursivityAnalysis>());


/**
 * This properties marks CFG and PCGNode corresponding to head of recursivity
 * nests. The argument is the PCG block corresponding to the main entry of a
 * recursivity nest: a recursivity nest may have several entries.
 * The choice of the main entry is implementation dependent.
 *
 * @par Features
 *	* otawa::RECURSIVITY_ANALYSIS
 *
 * @par Hooks
 *	* otawa::PCGNode
 *	* otawa::CFG
 *
 * @ingroup pcg
 */
p::id<PCGBlock *> RECURSE_HEAD("RECURSE_HEAD", nullptr);


/**
 * This property marks PCG edges and synthetic block corresponding to calls
 * entering a recursivity nest.
 *
 * @par Features
 *	* otawa::RECURSIVITY_ANALYSIS
 *
 * @par Hooks
 *	* otawa::PCGNode
 *	* otawa::CFG
 *
 * @ingroup pcg
 */
p::id<bool> RECURSE_ENTRY("otawa::RECURSE_ENTRY");


/**
 * This property marks PCG edges and synthetic block corresponding to calls
 * backing inside a recursivity nest.
 *
 * @par Features
 *	* otawa::RECURSIVITY_ANALYSIS
 *
 * @par Hooks
 *	* otawa::PCGNode
 *	* otawa::SynthBlock
 *
 * @ingroup pcg
 */
p::id<bool> RECURSE_BACK("otawa::RECURSE_BACK");

}	// otawa
