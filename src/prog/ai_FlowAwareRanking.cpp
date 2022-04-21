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
#include <otawa/cfg/features.h>
#include <otawa/ai/FlowAwareRanking.h>
#include <otawa/ai/SimpleWorkList.h>
#include <otawa/pcg/features.h>

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
 *		* @ref CFG_RANKING_FEATURE
 *
 * @ingroup ai
 */


/**
 */
p::declare FlowAwareRanking::reg = p::init("otawa::ai::FlowAwareRanking", Version(1, 1, 0))
	.make<FlowAwareRanking>()
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(RECURSIVITY_ANALYSIS)
	.provide(RANKING_FEATURE)
	.provide(CFG_RANKING_FEATURE);


/**
 */
FlowAwareRanking::FlowAwareRanking(p::declare& r): Processor(r) {
}


///
void *FlowAwareRanking::interfaceFor(const AbstractFeature& f) {
	if(&f == &CFG_RANKING_FEATURE)
		return static_cast<CFGRanking *>(this);
	else
		return nullptr;
}


///
int FlowAwareRanking::rankOf(Block *v) {
	return *RANK_OF(v);
}


/**
 */
void FlowAwareRanking::processWorkSpace(WorkSpace *ws) {

	// initialize work list
	Vector<Edge *> backs;
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	SimpleWorkList wl(coll);
	wl.put(coll->entry()->entry());
	RANK_OF(coll->entry()->entry()) = 0;

	// proceed until the work list is exhausted
	while(wl) {

		// record new ranking
		auto v = wl.get();
        if (logFor(LOG_BB))
            log << "\tworking on " << v << io::endl;
		int r = RANK_OF(v);
		if(logFor(LOG_INST))
			log << "\trank(" << r << ") for " << v << " (" << v->cfg() << ")\n";
		r++;

		// propagate to subprogram call
		if(v->isSynth() && v->toSynth()->callee() != nullptr) {
			auto ev = v->toSynth()->callee()->entry();
			if(!RANK_OF(ev).exists() || r > RANK_OF(ev)) {
				RANK_OF(ev) = r;
				wl.put(ev);
				continue;
			}
		}

		// propagate at subprogram return
		else if(v->isExit()) {
			for(auto c: v->cfg()->callers())
				if(!RECURSE_BACK(c))
					for(auto e = c->outs(); e(); e++) {
						if(!RANK_OF(e->sink()).exists()) {
							RANK_OF(e->sink()) = r;
							wl.put(e->sink());
						}
					}
			continue;
		}

		// propagate to successors
		for(auto e: v->outEdges())

			// do not propagate along exit edge
			if(LOOP_EXIT_EDGE(e))
				continue;

			// back edge propagate to exit edges
			else if(BACK_EDGE(e)) {
                if (EXIT_LIST(e->sink())== nullptr){
                    String err_msg = _ << "BackEdge: "
                            << e->source()->address() << "->" << e->sink()->address()
                            << " EXIT_LIST prop is not correctly set, check LoopInfoBuilder" << io::endl;
                    throw ProcessorException(*this, err_msg);
                }
				for(auto xe: **EXIT_LIST(e->sink()))
					backs.push(xe);
				while(backs) {
					auto ce = backs.pop();
					if(BACK_EDGE(ce))
						for(auto xe: **EXIT_LIST(ce->sink()))
							backs.push(xe);
					else
						if(r > RANK_OF(ce->sink())) {
							RANK_OF(ce->sink()) = r;
							wl.put(ce->sink());
						}
				}
			}

			// simple case
			else if(r > RANK_OF(e->sink())) {
				RANK_OF(e->sink()) = r;
				wl.put(e->sink());
			}
	}
}


///
void FlowAwareRanking::dump(WorkSpace *ws, Output& out) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	for(auto g: *coll) {
		out << "CFG " << g << io::endl;
		for(auto v = g->blocks(); v(); v++)
			out << "\t" << *v << ": " << *RANK_OF(*v) << io::endl;
	}
}


/*
 */
void FlowAwareRanking::destroy(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	for(auto v: coll->blocks())
		RANK_OF(v).remove();
}

} }

