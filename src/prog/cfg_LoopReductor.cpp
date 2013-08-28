/*
 *	$Id$
 *	LoopReductor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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

#include <otawa/proc/Processor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/Feature.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/cfg.h>
#include <otawa/util/Dominance.h>
#include <elm/genstruct/SortedSLList.h>
#include <otawa/cfg/LoopReductor.h>
#include <otawa/cfg/features.h>

/* a virer plus tard */

using namespace otawa::dfa;

namespace otawa {

/**
 * @class LoopReductor
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref COLLECTED_CFGS_FEATURE
 *
 * @par Provided features
 * @li @ref COLLECTED_CFGS_FEATURE
 * @li @ref REDUCED_LOOPS_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 *
 * @par Statistics
 * none
 *
 * @ingroup cfg
 */

LoopReductor::LoopReductor(bool _reduce_loops) : Processor((_reduce_loops ? "otawa::LoopReductor" : "otawa::SpanningTreeBuilder"), Version(1, 0, 0)), reduce_loops(_reduce_loops) {
	require(COLLECTED_CFG_FEATURE);
	invalidate(COLLECTED_CFG_FEATURE);
	provide(LOOP_HEADERS_FEATURE);
	if (reduce_loops) {
		provide(REDUCED_LOOPS_FEATURE);
	}
}

Identifier<BasicBlock*> LoopReductor::DUPLICATE_OF("otawa::LoopReductor::DUPLICATE_OF", 0);
Identifier<bool> LoopReductor::MARK("otawa::LoopReductor::MARK", false);

Identifier<dfa::BitSet*> LoopReductor::IN_LOOPS("otawa::LoopReductor::IN_LOOPS", 0);

void LoopReductor::processWorkSpace(otawa::WorkSpace *fw) {

	int idx = 0;

	const CFGCollection *orig_coll = INVOLVED_CFGS(fw);
	if (reduce_loops) {

		for (CFGCollection::Iterator cfg(*orig_coll); cfg; cfg++) {
			VirtualCFG *vcfg = new VirtualCFG(false);
			if (ENTRY_CFG(fw) == *cfg)
				ENTRY_CFG(fw) = vcfg;
			vcfgvec.add(vcfg);
			INDEX(vcfg) = INDEX(cfg);
			LABEL(vcfg) = LABEL(cfg);
			ENTRY(vcfg->entry()) = vcfg;

		}

		int i = 0;
		for (CFGCollection::Iterator cfg(*orig_coll); cfg; cfg++, i++) {
			VirtualCFG *vcfg = vcfgvec.get(i);

			idx = 0;
			INDEX(vcfg->entry()) = idx;
			vcfg->addBB(vcfg->entry());
			idx++;

			reduce(vcfg, cfg);

			INDEX(vcfg->exit()) = idx;
			vcfg->addBB(vcfg->exit());

			/* Renum basic blocks */
			idx = 0;
			for (CFG::BBIterator bb(vcfg); bb; bb++) {
				INDEX(bb) = idx;
				idx++;
			}
		}

	INVOLVED_CFGS(fw) = NULL;
	} else {
		for (CFGCollection::Iterator cfg(*orig_coll); cfg; cfg++) {

			Vector<BasicBlock*> *ancestors = new Vector<BasicBlock*>();

			for (CFG::BBIterator bb(cfg); bb; bb++) {
				IN_LOOPS(bb) = new dfa::BitSet(cfg->countBB());
			}

			/* Do the Depth-First Search, compute the ancestors sets, and mark loop headers */
			depthFirstSearch(cfg->entry(), ancestors);
			delete ancestors;
		}

	}

}

class EdgeDestOrder {
	public:
	static inline int compare(Edge *e1, Edge *e2) {
		if (e1->target() && e2->target() && e1->target()->address() && e2->target()->address()) {
			if (e1->target()->address() > e2->target()->address()) {
				return 1;
			} else {
				return -1;
			}
		} else {
			return 0;
		}
	}
};

void LoopReductor::depthFirstSearch(BasicBlock *bb, Vector<BasicBlock*> *ancestors) {
	ancestors->push(bb);
	MARK(bb) = true;


	SortedSLList<Edge*, EdgeDestOrder> successors;
	for (BasicBlock::OutIterator edge(bb); edge; edge++)
		successors.add(*edge);

	for (SortedSLList<Edge*, EdgeDestOrder>::Iterator edge(successors); edge; edge++) {
		if ((edge->kind() != Edge::CALL) && !edge->target()->isExit()){
			if (MARK(edge->target()) == false) {
				depthFirstSearch(edge->target(), ancestors);
			} else {
				if (ancestors->contains(edge->target())) {
					LOOP_HEADER(edge->target()) = true;
					BACK_EDGE(edge) = true;
					bool inloop = false;
					for (Vector<BasicBlock*>::Iterator member(*ancestors); member; member++) {
						if (*member == edge->target())
							inloop = true;
						if (inloop) {
							IN_LOOPS(member)->add(edge->target()->number());
						}

					}
				}
				/* GRUIIIIIIIIIK !!! pas performant, mais bon.. */
				for (dfa::BitSet::Iterator bit(**IN_LOOPS(edge->target())); bit; bit++) {
					bool inloop = false;
					for (Vector<BasicBlock*>::Iterator member(*ancestors); member; member++) {
						if (member->number() == *bit)
							inloop = true;
						if (inloop) {
							IN_LOOPS(member)->add(*bit);
						}

					}
				}
			}
		}
	}
	ancestors->pop();
}

void LoopReductor::reduce(VirtualCFG *vcfg, CFG *cfg) {

	HashTable<BasicBlock*,BasicBlock*> map;
	map.put(cfg->entry(), vcfg->entry());
	map.put(cfg->exit(),vcfg->exit());

	idx = 1;
	/* duplicate the basic blocks */
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		if (!bb->isEnd()) {
			BasicBlock *vbb = new VirtualBasicBlock(*bb);
			INDEX(vbb) = idx;
			idx++;
			map.put(bb, vbb);
			vcfg->addBB(vbb);
		}
	}
	INDEX(vcfg->exit()) = idx;
	idx++;

	/* connect edges */
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		for (BasicBlock::OutIterator edge(bb); edge; edge++) {
			if (edge->kind() == Edge::CALL) {
				BasicBlock *vsource = map.get(edge->source(), NULL);
				CFG *vcalled = vcfgvec.get(INDEX(edge->calledCFG()));
				ASSERT(vsource && vcalled);
				Edge *vedge = new Edge(vsource, vcalled->entry(), Edge::CALL);
				CALLED_CFG(vedge) = vcalled;

			} else {
				BasicBlock *vsource = map.get(edge->source(), NULL);
				BasicBlock *vtarget = map.get(edge->target(), NULL);
				ASSERT(vsource && vtarget);
				new Edge(vsource, vtarget, edge->kind());
			}
		}
	}


	Vector<BasicBlock*> *ancestors = new Vector<BasicBlock*>();

	for (CFG::BBIterator bb(vcfg); bb; bb++) {
		IN_LOOPS(bb) = new dfa::BitSet(vcfg->countBB());
	}

	/* Do the Depth-First Search, compute the ancestors sets, and mark loop headers */
	depthFirstSearch(vcfg->entry(), ancestors);

	/* Collect all loop headers in a bitset */
/*
	for (CFG::BBIterator bb(vcfg); bb; bb++)
		if (LOOP_HEADER(bb)) {
			realhdr.add(bb->number());
		}
		*/
	/*
	HashTable<int, BasicBlock*> hdrmap;
	Vector<BasicBlock*> dups;
	*/


	bool done = false;
	while (!done) {
		done = true;
		for (CFG::BBIterator bb(vcfg); bb; bb++) {
			Vector<Edge*> toDel;
			BasicBlock *duplicate = NULL;
			for (BasicBlock::InIterator edge(bb); edge; edge++) {

				/* compute loops entered by the edge */
				dfa::BitSet enteredLoops(**IN_LOOPS(bb));
				enteredLoops.remove(**IN_LOOPS(edge->source()));

				/* The edge is a regular entry if it enters one loop, and edge->target() == loop header */
				if (!((enteredLoops.count() == 0) || ((enteredLoops.count() == 1) && (enteredLoops.contains(bb->number()))))) {
					if (!duplicate) {
						duplicate = new VirtualBasicBlock(bb);
						ASSERT(DUPLICATE_OF(bb) == NULL);
						DUPLICATE_OF(bb) = duplicate;
						INDEX(duplicate) = idx;
						idx++;
						vcfg->addBB(duplicate);
						IN_LOOPS(duplicate) = new dfa::BitSet(**IN_LOOPS(edge->source()));
						for (BasicBlock::OutIterator outedge(bb); outedge; outedge++) {
							if (DUPLICATE_OF(outedge->target())) {
								new Edge(duplicate, DUPLICATE_OF(outedge->target()), outedge->kind());
							} else {
								new Edge(duplicate, outedge->target(), outedge->kind());
							}
						}

					}
					done = false;

					new Edge(edge->source(), duplicate, edge->kind());

					toDel.add(edge);
				}
			}
			for (Vector<Edge*>::Iterator edge(toDel); edge; edge++)
				delete *edge;
		}
	}

	delete ancestors;
}


static SilentFeature::Maker<LoopReductor> REDUCED_LOOPS_MAKER;
/**
 *
 */
SilentFeature REDUCED_LOOPS_FEATURE ("otawa::REDUCED_LOOPS_FEATURE", REDUCED_LOOPS_MAKER);



} // otawa
