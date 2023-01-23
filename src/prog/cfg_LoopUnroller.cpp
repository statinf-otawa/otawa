/*
 *	$Id$
 *	LoopUnroller processor interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <elm/io.h>
#include <elm/data/List.h>
#include <elm/data/Vector.h>
#include <elm/data/HashMap.h>
#include <elm/data/VectorQueue.h>
#include <elm/util/Pair.h>
#include <otawa/cfg.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/LoopUnroller.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/IPET.h>
#include <otawa/cfg/features.h>
#include "../../include/otawa/flowfact/FlowFactLoader.h"

using namespace otawa;
using namespace elm;



namespace otawa {

// Tool identifier to record exit to following header
// while header has not been cloned.
// Put on the target header.
static Identifier<List<Pair<Block *, Edge *> > > DELAYED_EDGE("");

/**
 * This feature that the loops have been unrolled at least once.
 *
 * @par Configuration
 *	* @ref UNROLL_THIS -- mark loops to unroll
 *
 * @par Properties
 * @li @ref UNROLLED_FROM
 *
 * @ingroup cfg
 */
p::feature UNROLLED_LOOPS_FEATURE ("otawa::UNROLLED_LOOPS_FEATURE", new Maker<LoopUnroller>());


/**
 * This property is used by @ref UNROLLED_LOOPS_FEATURE and has to be put on
 * header blocks of loops to select if the corresponding loop has to be
 * unrolled or not. Its default value is true to unroll all loops.
 *
 * @par Feature
 *	* @ref UNROLLED_LOOPS_FEATURE
 */
p::id<bool> UNROLL_THIS("otawa::UNROLL_THIS", true);


/**
 * Put on the header ex-header of a loop, this property gives the BB of the unrolled loop.
 *
 * @par Hooks
 * @li @ref BasicBlock
 *
 * @ingroup cfg
 */
Identifier<Block *> UNROLLED_FROM("otawa::UNROLLED_FROM", 0);


/**
 * @class LoopUnroller
 *
 * This processor unrolls the first iteration of each loop
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref FLOW_FACT_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_CFGS_FEATURE

 *
 * @par Provided features
 * @li @ref COLLECTED_CFGS_FEATURE
 * @li @ref UNROLLED_LOOPS_FEATURE
 *
 * @par Statistics
 * none
 *
 * @ingroup cfg
 */

p::declare LoopUnroller::reg = p::init("otawa::LoopUnroller", Version(2, 2, 0))
	.base(CFGTransformer::reg)
	.maker<LoopUnroller>()
	.use(DOMINANCE_FEATURE)
	.use(LOOP_HEADERS_FEATURE)
	.use(LOOP_INFO_FEATURE)
	.use(COLLECTED_CFG_FEATURE)
	.provide(UNROLLED_LOOPS_FEATURE);


/**
 */
LoopUnroller::LoopUnroller(p::declare& r): CFGTransformer(r), coll(new CFGCollection()), idx(0) {
}


/**
 */
void LoopUnroller::transform(CFG *cfg, CFGMaker& maker) {
	unroll(cfg, nullptr, &maker);
}


/**
 * Test if the given loop has to be unrolled.
 * @param h		Header of the loop.
 * @return		True if the loop has to be unrolled, false else.
 */
bool LoopUnroller::unrolls(Block *h) {
	return *UNROLL_THIS(h);
}



/**
 */
void LoopUnroller::unroll(otawa::CFG *cfg, Block *header, CFGMaker *vcfg) {
	if(logFor(LOG_FUN))
		log << "\tunrolling " << cfg << " for " << (header ? header : cfg->entry()) << io::endl;

	VectorQueue<Block*> workList;								// workList := { }
	VectorQueue<Block*> loopList;								// loopList := { }
	Vector<Block*> doneList;									// doneList := { }
	typedef Vector<Pair<Block *, Edge *> > BackEdgePairVector;	// backEdges := { }

	BackEdgePairVector backEdges;
	bool dont_unroll = false;
	Block *unrolled_from = nullptr;
	int start;

	// avoid unrolling loops with LOOP_COUNT of 0, since it would create a LOOP_COUNT of -1 for the non-unrolled part of the loop
	if(header != nullptr)
		dont_unroll = !unrolls(header) || MAX_ITERATION(header) == 0;
	start = dont_unroll ? 1 : 0;

	// duplicate the loop body
	// for in in [start, 1] do
	for(int i = start; (i < 2 && header != nullptr) || i < 1; i++) {
		ASSERT(workList.isEmpty());
		ASSERT(loopList.isEmpty());

		Vector<Block*> bbs;											// bbs := {}
		workList.put(header != nullptr ? header : cfg->entry());	// workList := { header if any, entry else }
		doneList.clear();
		doneList.add(header != nullptr ? header : cfg->entry());	// doneList := { header if any, entry else }

		// duplicate the blocks
		while (!workList.isEmpty()) {		// while workList /= { } do

			// current::workList := workList
			Block *current = workList.get();

			// record sub-loop headers
			if(LOOP_HEADER(current) && current != header) {

				// save current
				loopList.put(current);

				// add exit edges destinations to the worklist
				cerr << "DEBUG: " << current << io::endl;
				for (Vector<Edge*>::Iter exitedge(**EXIT_LIST(current)); exitedge(); exitedge++)
					if (!doneList.contains(exitedge->target())) {
						workList.put(exitedge->target());
						doneList.add(exitedge->target());
					}
			}

			// clone the block
			else {
				Block *new_bb = clone(current);

				// add delayed edge
				if(i == start && current->hasProp(DELAYED_EDGE)) {
					for(List<Pair<Block *, Edge *> >::Iter d(DELAYED_EDGE(current)); d(); d++)
						clone((*d).fst, (*d).snd, new_bb);
					current->removeProp(DELAYED_EDGE);
				}

				// for header, record origin
				if (current == header && !dont_unroll) {
					if (i == 0)
						unrolled_from = new_bb;
					else
						UNROLLED_FROM(new_bb) = unrolled_from;
				}

				// record mapping
				map.put(current, new_bb);
				bbs.add(current);

				// add successors which are in loop (including possible sub-loop headers)
				// for (current, sink) /\ sink /= entry in E do
				for(Block::EdgeIter outedge = current->outs(); outedge(); outedge++) {
					if (outedge->target() == cfg->exit())
						continue;

					// block inside current loop
					// if sink in loop(header) /\ sink not in doneList
					if(ENCLOSING_LOOP_HEADER(outedge->target()) == header
					&& !doneList.contains(outedge->target())) {
						// workList U= sink
						// doneList U= sink
						workList.put(outedge->target());
						doneList.add(outedge->target());
					}

					// connect exit edge case
					// if LOOP_EXIT_EDGE(outedge) then
					if(LOOP_EXIT_EDGE(*outedge)) {
						Block *vdst = map.get(outedge->sink(), 0);

						// simple exit
						if(vdst)
							clone(new_bb, *outedge, vdst);

						// delayed exit
						else if(i == start)
							(*DELAYED_EDGE(outedge->sink())).add(pair(new_bb, *outedge));
					}
				}
			}
		}

		// duplicate sub-loops
		// for loop in loopList do
		while(!loopList.isEmpty()) {
			Block *loop = loopList.get();
			unroll(cfg, loop, vcfg);
		}

		// connect the internal edges for the current loop
		// for bb in bbs do
		for(Vector<Block*>::Iter bb(bbs); bb(); bb++)
			// for (bb, sink) in E
			for(Block::EdgeIter outedge = bb->outs(); outedge(); outedge++) {

				// nothing ti di with exit edge, sub-loop headers and CFG exit
				// /\ not EXIT_EDGE(bb, sink)
				if(LOOP_EXIT_EDGE(*outedge))
					continue;
				// /\ not(LOOP_HEADER(sink)) /\ sink /= header)
				if(LOOP_HEADER(outedge->target()) && outedge->target() != header)
					continue;
				// /\ sink /= exit do
				if(outedge->target() == cfg->exit())
					continue;

				// find mapped source and sink blocks
				Block *vsrc = map.get(*bb, 0);
				ASSERT(vsrc);
				Block *vdst = map.get(outedge->target(), 0);
				ASSERT(vdst);

				// make the edge
				// if sink /= header \/ i = 1
				if(outedge->target() != header || i == 1)
					// E' U= (map[bb], map[sink])
					clone(vsrc, *outedge, vdst);
				else
					// backEdges U= {(map[bb], (bb, sink))}
					backEdges.add(pair(vsrc, *outedge));
			}

		// first unroll iteration: virtual entry edges
		// if i == start /\ header /= null then
		if(i == start && header)
				// for (src, header) in E /\ header not dom src /\ src /= entry do
				for(Block::EdgeIter inedge = header->ins(); inedge(); inedge++) {
					if(Dominance::dominates(header, inedge->source()))
						continue; 	// skip back edges
					if(inedge->source() == cfg->entry())
						continue;	// skip edge from CFG entry
					Block *vsrc = map.get(inedge->source());
					Block *vdst = map.get(header);
					// E' U= {(map[src], map[header])}
					clone(vsrc, *inedge, vdst);
				}

		// other unroll iterations: connect virtual backedge from the first to other iterations
		else
			// for (src', e) in backEdges do
			for(BackEdgePairVector::Iter b(backEdges); b(); b++) {
				Block *vdst = map.get(header);
				// E' U= {(src', map[header])}
				clone((*b).fst, (*b).snd, vdst);
			}

	}

	// top-level CFG duplication: add exit edges
	if(!header)
		for(BasicBlock::EdgeIter inedge = cfg->exit()->ins(); inedge(); inedge++) {
			Block *vsrc = map.get(inedge->source(), 0);
			ASSERT(vsrc);
			clone(vsrc, *inedge, vcfg->exit());
		}

}

} // otawa

