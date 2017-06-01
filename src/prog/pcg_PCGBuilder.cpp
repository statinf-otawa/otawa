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

p::declare PCGBuilder::reg = p::init("otawa::PCGBuilder", Version(2, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.provide(PCG_FEATURE)
	.extend<Processor>()
	.make<PCGBuilder>();


/**
 */
PCGBuilder::PCGBuilder(p::declare& r): Processor(r) {
}


/**
 */
void PCGBuilder::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = otawa::INVOLVED_CFGS(ws);

	// initialize the PCG
	CFGCollection::Iter cfg(coll);
	PCGBlock *b = new PCGBlock(cfg);
	map.put(cfg, b);
	otawa::PCG *pcg = new otawa::PCG(b);
	pcg->add(b);

	// build a block for each CFG
	for(cfg++; cfg; cfg++) {
		PCGBlock *b = new PCGBlock(cfg);
		map.put(cfg, b);
		pcg->add(b);
	}

	// build the links
	for(CFGCollection::Iter cfg(coll); cfg; cfg++)
		for(CFG::CallerIter call = cfg->callers(); call; call++)
			new PCGEdge(map.get(call->caller()), call, map.get(cfg));

	// install all
	track(otawa::PCG_FEATURE, PROGRAM_CALL_GRAPH(ws) = pcg);
	map.clear();
}


/**
 * This feature ensure that a PCG is provided.
 *
 * @par Properties
 * @li @ref PROGRAM_CALL_GRAPH
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
 */
p::id<PCG *> PROGRAM_CALL_GRAPH("otawa::PROGRAM_CALL_GRAPH", 0);

}	// otawa
