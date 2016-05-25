/*
 *	LoopReductor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-16, IRIT UPS.
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

#include <elm/genstruct/SortedSLList.h>
#include <otawa/cfg.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/Processor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/LoopReductor.h>

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

p::declare LoopReductor::reg = p::init("otawa::LoopReductor", Version(2, 0, 0))
	.require(COLLECTED_CFG_FEATURE)
	.invalidate(COLLECTED_CFG_FEATURE)
	.provide(LOOP_HEADERS_FEATURE)
	.provide(COLLECTED_CFG_FEATURE)
	.provide(REDUCED_LOOPS_FEATURE)
	.make<LoopReductor>();

LoopReductor::LoopReductor(p::declare& r)
	: Processor(r), idx(0)
{
}

Identifier<Block*> LoopReductor::DUPLICATE_OF("otawa::LoopReductor::DUPLICATE_OF", 0);
Identifier<bool> LoopReductor::MARK("otawa::LoopReductor::MARK", false);

Identifier<dfa::BitSet*> LoopReductor::IN_LOOPS("otawa::LoopReductor::IN_LOOPS", 0);

/**
 */
void LoopReductor::processWorkSpace(otawa::WorkSpace *ws) {
	const CFGCollection *orig_coll = INVOLVED_CFGS(ws);

	// make the makers
	for(CFGCollection::Iterator cfg(*orig_coll); cfg; cfg++) {
		CFGMaker *vcfg = new CFGMaker(cfg->first());
		vcfgvec.add(vcfg);
	}

	// reduce loops
	int i = 0;
	for(CFGCollection::Iterator cfg(*orig_coll); cfg; cfg++, i++) {
		CFGMaker *vcfg = vcfgvec.get(i);
		reduce(vcfg, cfg);
	}

}


/**
 */
void LoopReductor::cleanup(WorkSpace *ws) {
	CFGCollection *new_coll = new CFGCollection();
	for(int i = 0; i < vcfgvec.count(); i++) {
		CFG *cfg = vcfgvec[i]->build();
		new_coll->add(cfg);
		if(i == 0)
			addRemover(COLLECTED_CFG_FEATURE, ENTRY_CFG(ws) = cfg);
		delete vcfgvec[i];
	}
	track(COLLECTED_CFG_FEATURE, INVOLVED_CFGS(ws) = new_coll);
}



// sort edge by increasing target address
class EdgeDestOrder {
public:
	static inline int compare(Edge *e1, Edge *e2) {
		if(e1->target() && e2->target() && e1->target()->address() && e2->target()->address())
			if (e1->target()->address() > e2->target()->address())
				return 1;
			else
				return -1;
		else
			return 0;
	}
};


/**
 * Perform the depth-first search
 * @param bb		Root of the spanning tree.
 * @param ancestors	Used to store ancestors.
 */
void LoopReductor::depthFirstSearch(Block *bb, Vector<Block *> *ancestors) {
	ancestors->push(bb);
	MARK(bb) = true;

	// S = { v / (bb, v) in E }
	SortedSLList<Edge *, EdgeDestOrder> successors;
	for(Block::EdgeIter edge = bb->outs(); edge; edge++)
		successors.add(*edge);

	// foreach (bb, v) in S
	for(SortedSLList<Edge*, EdgeDestOrder>::Iterator edge(successors); edge; edge++) {
		if(!edge->target()->isExit()) {

			// if not MARK(v) go down
			if (MARK(edge->target()) == false)
				depthFirstSearch(edge->target(), ancestors);

			// else assert MARK(v)
			else {

				// is it a back edge?
				if(ancestors->contains(edge->target())) {
					LOOP_HEADER(edge->target()) = true;
					BACK_EDGE(edge) = true;
					//cerr << "DEBUG:\tback edge " << *edge << io::endl;

					// foreach w in S[v, bb] do IN_LOOPS(w) <- IN_LOOPS U { v }
					bool inloop = false;
					for(Vector<Block*>::Iterator member(*ancestors); member; member++) {
						if(*member == edge->target())
							inloop = true;
						if(inloop)
							IN_LOOPS(member)->add(edge->target()->index());
					}
				}

				// foreach t in IN_LOOPS(v) do
				dfa::BitSet *il_v = IN_LOOPS(edge->target());
				ASSERT(il_v);
				for (dfa::BitSet::Iterator bit(*il_v); bit; bit++) {
					bool inloop = false;

					// foreach w in S[v, bb] do IN_LOOPS[w] <- IN_LOOPS[w] U { t }
					for (Vector<Block*>::Iterator member(*ancestors); member; member++) {
						if (member->index() == *bit)
							inloop = true;
						if (inloop)
							IN_LOOPS(member)->add(*bit);
					}
				}
			}
		}
	}

	// go up
	ancestors->pop();
}


/**
 * Duplicate the given block.
 * @param maker		CFG maker to use.
 * @param b			Block to duplicate (must not be an end).
 * @param duplicate	If true, the cloned block has already been cloned.
 * @return			Duplicated block.
 */
Block *LoopReductor::clone(CFGMaker& maker, Block *b, bool duplicate) {

	if(b->isBasic()) {
		BasicBlock *bb = b->toBasic();
		genstruct::Table<Inst *> insts(new Inst *[bb->count()], bb->count());
		int j = 0;
		for(BasicBlock::InstIter i = bb->insts(); i; i++, j++)
			insts[j] = i;
		BasicBlock *nbb = new BasicBlock(insts);
		maker.add(nbb);
		return nbb;
	}

	else if(b->isSynth()) {
		SynthBlock *sb = b->toSynth();
		SynthBlock *nsb = new SynthBlock();
		if(!sb->callee())
			maker.call(nsb, 0);
		else if(duplicate)
			maker.call(nsb, sb->callee());
		else
			maker.call(nsb, *vcfgvec[sb->callee()->index()]);
		return nsb;
	}

	else {
		ASSERT(false);
		return 0;
	}
}


/**
 * Reduce irregular loops.
 */
void LoopReductor::reduce(CFGMaker *maker, CFG *cfg) {

	HashTable<Block *, Block *> map;
	map.put(cfg->entry(), maker->entry());
	map.put(cfg->exit(),maker->exit());
	if(cfg->unknown())
		map.put(cfg->unknown(), maker->unknown());

	// duplicate the basic blocks
	map.put(cfg->entry(), maker->entry());
	for(CFG::BlockIter b = cfg->blocks(); b; b++)
		if(!b->isEnd()) {
			Block *nb = clone(*maker, b);
			map.put(b, nb);
		}
	map.put(cfg->exit(), maker->exit());

	// connect edges
	for(CFG::BlockIter b = cfg->blocks(); b; b++) {
		for(Block::EdgeIter edge = b->outs(); edge; edge++) {
			Block *vsource = map.get(edge->source(), 0);
			Block *vtarget = map.get(edge->target(), 0);
			ASSERT(vsource && vtarget);
			Edge *nedge = new Edge(edge->flags());
			maker->add(vsource, vtarget, nedge);
		}
	}

	// prepare irregular analysis
	Vector<Block*> *ancestors = new Vector<Block*>();
	//for(CFG::BlockIter bb = maker->blocks(); bb; bb++)
	//	IN_LOOPS(bb) = new dfa::BitSet(maker->count());

	// do the Depth-First Search, compute the ancestors sets, and mark loop headers
	//depthFirstSearch(maker->entry(), ancestors);

	// perform the transformation
	bool done = false;
	while(!done) {
		//cerr << "\nDEBUG: new pass: " << maker->count() << "\n";
		done = true;

		// build IN_LOOPS
		ancestors->clear();
		for(CFG::BlockIter bb = maker->blocks(); bb; bb++)
			IN_LOOPS(bb) = new dfa::BitSet(maker->count());
		depthFirstSearch(maker->entry(), ancestors);
		for(CFG::BlockIter bb = maker->blocks(); bb; bb++) {
			MARK(bb) = false;
			DUPLICATE_OF(bb) = 0;
		}

		// foreach b in V do
		for(CFG::BlockIter b = maker->blocks(); b; b++) {
			Vector<Edge*> toDel;

			// foreach (v, b) in E do
			for(Block::EdgeIter edge = b->ins(); edge; edge++) {

				// compute loops entered by the edge
				// enteredLoops = IN_LOOPS(b) \ IN_LOOPS(v)
				dfa::BitSet enteredLoops(**IN_LOOPS(b));
				enteredLoops.remove(**IN_LOOPS(edge->source()));

				// the edge is a irregular entry if it enters one loop, and edge->target() == loop header
				// if |enteredLoops| > 1 /\ b in enteredLoops then
				if(!((enteredLoops.count() == 0) || ((enteredLoops.count() == 1) && (enteredLoops.contains(b->index()))))) {

					// d <- duplicate(v)
					Block *duplicate = DUPLICATE_OF(b);
					if(!duplicate) {
						done = false;

						// DUPLICATE_OF(b) <- d
						duplicate = clone(*maker, b, true);
						ASSERT(DUPLICATE_OF(b) == 0);
						DUPLICATE_OF(b) = duplicate;

						// IN_LOOPS(d) <- IN_LOOPS(v)
						IN_LOOPS(duplicate) = new dfa::BitSet(**IN_LOOPS(edge->source()));

						// E <- E U { (d, DUPLICATE_OF(w)) / (b, w) in E }
						for(Block::EdgeIter outedge = b->outs(); outedge; outedge++) {
							if(DUPLICATE_OF(outedge->target())) {
								Edge *nedge = new Edge(outedge->flags());
								maker->add(duplicate, DUPLICATE_OF(outedge->target()), nedge);
							} else {
								Edge *nedge = new Edge(outedge->flags());
								maker->add(duplicate, outedge->target(), nedge);
							}
						}

						// E <- E U { (v, d) } \ { (v, b) }
						Edge *nedge = new Edge(edge->flags());
						maker->add(edge->source(), duplicate, nedge);
						toDel.add(edge);
					}

				}
			}

			for (Vector<Edge*>::Iterator edge(toDel); edge; edge++)
				delete *edge;
		}
	}

	delete ancestors;
}


/**
 * Ensure that no more irregular loop remains in the program representation.
 */
p::feature REDUCED_LOOPS_FEATURE ("otawa::REDUCED_LOOPS_FEATURE", p::make<LoopReductor>());

} // otawa
