/*
 *	FlowAwareRanking class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
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

#include <elm/compare.h>
#include <elm/data/ListQueue.h>
#include <otawa/cfg/features.h>
#include <otawa/ai/FlowAwareRanking.h>

namespace otawa { namespace ai {

/**
 * @class FlowAwareRanking
 * Build a ranking for each block of CFG collection to speed up static
 * analyzes based on the following principle:
 *	* except for a loop header, a block must be processed after its predecessor,
 *	* a block after the exit of a loop must be processed after the loop blocks,
 *	* a block after a subprogram call must be processed after blocks of the called subprogram.
 *
 *	@par Required features
 *		* @ref COLLECTED_CFGS_FEATURE
 *		* @ref LOOP_INFO_FEATURE
 *
 *	@par Provided features
 *		* @ref RANKING_FEATURE
 *
 * @ingroup ai
 */


/**
 */
p::declare FlowAwareRanking::reg = p::init("otawa::ai::FlowAwareRanking", Version(1, 0, 0))
	.make<FlowAwareRanking>()
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.provide(RANKING_FEATURE);


/**
 */
FlowAwareRanking::FlowAwareRanking(p::declare& r): Processor(r) {
}


/**
 */
void FlowAwareRanking::processWorkSpace(WorkSpace *ws) {

	// initialize work list
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	ListQueue<Pair<Block *, int> > wl;
	wl.put(pair(coll->entry()->entry(), 0));

	// proceed until the work list is exhausted
	while(wl) {

		// record new ranking
		Block *v = wl.head().fst;
		int r = wl.head().snd;
		wl.get();
		RANK_OF(v) = r;
		cerr << "DEBUG: propagating after " << v << " with rank " << r << io::endl;
		r++;

		// propagate at subprogram call
		if(v->isSynth() && v->toSynth()->callee() != nullptr && !RANK_OF(v->toSynth()->callee()->entry()).exists()) {
			if(r > RANK_OF(v->toSynth()->callee()->entry()))
				wl.put(pair(v->toSynth()->callee()->entry(), r));
		}

		// propagate at subprogram return
		else if(v->isExit())
			for(auto c = v->cfg()->callers(); c; c++)
				for(auto e = c->outs(); e; e++)
					if(r > RANK_OF(e->sink()))
						wl.put(pair(e->sink(), r));

		// propagate to successors
		for(auto e = v->outs(); e; e++)

			// do not propagate along exit edge
			if(LOOP_EXIT_EDGE(e))
				continue;

			// back edge propagate to exit edges
			else if(BACK_EDGE(e)) {
				for(auto f = ***EXIT_LIST(e->sink()); f; f++)
					if(r > RANK_OF(f->sink()))
						wl.put(pair(f->sink(), r));
			}

			// simple case
			else if(r > RANK_OF(e->sink()))
				wl.put(pair(e->sink(), r));
	}

	if(logFor(LOG_BLOCK)) {
		for(auto g = coll->items(); g; g++) {
			log << "\tCFG " << *g << io::endl;
			for(auto v = g->blocks(); v; v++)
				log << "\t\t" << *v << ": " << *RANK_OF(v) << io::endl;
		}
	}
}

/*
 */
void FlowAwareRanking::destroy(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	for(auto v = coll->blocks(); v; v++)
		RANK_OF(v).remove();
}

} }

