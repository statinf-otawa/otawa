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

#include <elm/data/HashMap.h>
#include <elm/data/SortedList.h>
#include <elm/util/misc.h>
#include <otawa/cfg.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/Processor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/LoopReductor.h>

#include <otawa/display/Displayer.h>
#include "../../include/otawa/display/CFGDecorator.h"

using namespace otawa::dfa;

extern "C" void exit(int);

//#define DO_DEBUG
#ifdef DO_DEBUG
#	define DEBUG(s)	cerr << "DEBUG: " << s
#else
#	define DEBUG(s)
#endif

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

p::declare LoopReductor::reg = p::init("otawa::LoopReductor", Version(2, 1, 0))
	.use(COLLECTED_CFG_FEATURE)
	.invalidate(COLLECTED_CFG_FEATURE)
	.provide(COLLECTED_CFG_FEATURE)
	.provide(REDUCED_LOOPS_FEATURE)
	.make<LoopReductor>()
	.extend<CFGProvider>();

LoopReductor::LoopReductor(p::declare& r)
	: CFGProvider(r), coll(nullptr)
{
}

Identifier<Block*> LoopReductor::DUPLICATE_OF("otawa::LoopReductor::DUPLICATE_OF", 0);
Identifier<bool> LoopReductor::MARK("otawa::LoopReductor::MARK", false);

Identifier<dfa::BitSet*> LoopReductor::IN_LOOPS("otawa::LoopReductor::IN_LOOPS", 0);

#ifdef DO_DEBUG
	static Identifier<bool> TO_DUMP("", false);
	static void displayCFG(WorkSpace *ws, CFG *cfg, string tag) {
		static display::Provider *prov = 0;
		if(!prov)
			prov = display::Provider::get();
		display::CFGDecorator deco(ws);
		deco.display_assembly = false;
		deco.display_props = false;
		display::DisplayedCFG dcfg(*cfg);
		display::Displayer *disp = prov->make(dcfg, deco, display::OUTPUT_RAW_DOT);
		disp->setPath(sys::Path(_ << "graphs/" << cfg->label() << "." << tag << ".dot"));
		disp->process();
		delete disp;
	}
#endif


/**
 */
void LoopReductor::processWorkSpace(otawa::WorkSpace *ws) {
	const CFGCollection *orig_coll = INVOLVED_CFGS(ws);

	// make the makers
	for(CFGCollection::Iter cfg(*orig_coll); cfg(); cfg++) {
		CFGMaker *vcfg = new CFGMaker(cfg->first(), true);
		vcfgvec.add(vcfg);
	}

	// reduce loops
	int i = 0;
	for(CFGCollection::Iter g(*orig_coll); g(); g++, i++) {
		//displayCFG(ws, cfg, "old");
		if(logFor(LOG_FUN))
			log << "\treducing function " << *g << io::endl;

		// duplicate graph
		CFGMaker& maker = *vcfgvec.get(i);
		HashMap<Block *, Block *> map;
		for(CFG::BlockIter v = g->blocks(); v(); v++)
			map.put(*v, clone(maker, *v));
		for(CFG::BlockIter v = g->blocks(); v(); v++)
			for(Block::EdgeIter e = v->outs(); e(); e++)
				maker.add(map.get(*v), map.get(e->sink()), new Edge(e->flags()));

		// iterate until irreducible loops are processed
		Vector<dfa::BitSet *> L;
		bool reduced;
		int it = 0;
		do {
			if(logFor(LOG_FUN))
				log << "\t\tPASS " << it << io::endl;
			computeInLoops(maker, L);
			reduced = reduce(maker, L);
	#		ifdef DO_DEBUG
				if(TO_DUMP(maker))
					displayCFG(ws, g, _ << "old-" << it);
	#		endif

			// cleanup
			for(CFG::BlockIter v = maker.blocks(); v(); v++) {
				delete IN_LOOPS(*v);
				v->removeProp(IN_LOOPS);
			}
			for(loops_t::Iter l(L); l(); l++)
				delete *l;

			it++;
			//if(it > 3)
			//	break;
		} while(reduced);
		if(logFor(LOG_FUN) && g->count() != maker.count())
			log << "\t\tirreducible loops of " << *g << " removed (before: "
				<< g->count() << " blocks, after: " << maker.count() << " blocks)\n";
	}

	// record the new collection
	coll = new CFGCollection();
	for(int i = 0; i < vcfgvec.count(); i++) {
#		ifdef DO_DEBUG
			bool to_dump = TO_DUMP(vcfgvec[i]);
#		endif
		CFG *cfg = vcfgvec[i]->build();
		coll->add(cfg);
#		ifdef DO_DEBUG
			if(to_dump)
				displayCFG(ws, cfg, "new");
#endif
		delete vcfgvec[i];
	}
	setCollection(coll);
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
		Array<Inst *> insts(bb->count(), new Inst *[bb->count()]);
		int j = 0;
		for(BasicBlock::InstIter i = bb->insts(); i(); i++, j++)
			insts[j] = *i;
		BasicBlock *nbb = new BasicBlock(insts);
		maker.add(nbb);
		return nbb;
	}

	else if(b->isSynth()) {
		SynthBlock *sb = b->toSynth();
		SynthBlock *nsb = new SynthBlock();
		if(sb->callee() == nullptr)
			maker.call(nsb, nullptr);
		else if(duplicate)
			maker.call(nsb, sb->callee());
		else
			maker.call(nsb, *vcfgvec[sb->callee()->index()]);
		return nsb;
	}

	else if(b->isEntry())
		return maker.entry();
	else if(b->isExit())
		return maker.exit();
	else if(b->isUnknown())
		return maker.unknown();
	else if(b->isPhony()) {
		PhonyBlock *nb = new PhonyBlock();
		maker.add(nb);
		return nb;
	}
	else {
		ASSERT(false);
		return 0;
	}

}


/**
 * Reduce irregular loops.
 * @param G		Graph to process.
 * @param L		List of header loops.
 * @return		True if an irregular loop has been reduced, false else.
 */
bool LoopReductor::reduce(CFGMaker& G, loops_t& L) {

	// for l ∈ L do
	for(loops_t::Iter l(L); l(); l++)

		// if |l| > 1 then
		if(l->count() > 1) {
#			ifdef DO_DEBUG
				TO_DUMP(G) = true;
#			endif

			// compute best header and nodes to duplicate	O(|L| × |N|)
			if(logFor(LOG_FUN))
				log << "\t\tirregular loop headed by " << **l << io::endl;

			// bh ← 0; bc ← ∅
			dfa::BitSet s1(G.count());
			dfa::BitSet s2(G.count());
			Block *bh = 0;
			dfa::BitSet *bs = &s1, *cs = &s2;

			// for h ∈ l do
			for(dfa::BitSet::Iterator hi(**l); hi(); hi++) {
				Block *h = G.at(*hi);

				// C ← ∅
				dfa::BitSet& C = *cs;
				C.empty();

				// W ← l \ h; C ← l \ h
				Vector<Block *> W;
				for(dfa::BitSet::Iterator ch(**l); ch(); ch++)
					if(*hi != *ch) {
						W.add(G.at(*ch));
						C.add(*ch);
					}

				// while W ≠ ∅ ∧ |C| < |bc| do
				while(W && (!bh || C.count() < bs->count())) {

					// let v = get(W) in
					Block *v = W.pop();

					// for (v, w) ∈ E do
					for(Block::EdgeIter e = v->outs(); e(); e++) {
						Block *w = e->sink();

						// if w ∉ l ∧ w ∉ C ∧ IL(h) ⊆ IL(w) then
						if(!l->contains(w->index())
						&& !C.contains(w->index())
						&& IN_LOOPS(w)->includes(**IN_LOOPS(h))) {

								// W ← W ∪ { w }
								W.add(w);

								// C ← C ∪ { w }
								C.add(w->index());
						}

					}
				}

				// if |C| > |bs| then
				if(logFor(LOG_BLOCK))
					log << "\t\t\theader " << *hi << " would consume at least " << C.count() << " blocks.\n";
				if(!bh || C.count() < bs->count()) {

					// bc ← C
					swap(bs, cs);

					// bh ← h
					bh = h;
				}
			}
			if(logFor(LOG_FUN))
				log << "\t\theader " << bh << " chosen as header of " << **l << io::endl;

			// perform the duplication	O(|BC|)
			Block *map[G.count()];
			for(CFG::BlockIter v = G.blocks(); v(); v++)
				map[v->index()] = *v;

			// for v ∈  bs do
			for(dfa::BitSet::Iterator v(*bs); v(); v++) {

				// let v' ← duplicate(v) in
				// V' ← V' ∪ { v' }
				Block *nv = clone(G, G.at(*v), true);
				if(logFor(LOG_BLOCK))
					log << "\t\t\t" << G.at(*v) << " duplicated as " << nv << io::endl;

				// σ ← λw . if w = v then v' else σ(w)
				map[*v] = nv;
			}

			// for v ∈ bs do
			for(dfa::BitSet::Iterator v(*bs); v(); v++) {

				// for (v, w)  ∈ E do
				for(Block::EdgeIter e = G.at(*v)->outs(); e(); e++) {

					// E' ← E' ∪ { (σ(v), σ(w)) }
					G.add(map[e->source()->index()], map[e->sink()->index()], new Edge(e->flags()));
					if(logFor(LOG_BLOCK))
						log << "\t\t\tadd edge " << map[e->source()->index()] << " -> " << map[e->sink()->index()] << io::endl;
				}
			}

			// for v ∈ l \ { bh } do
			Vector<Edge *> D;
			for(dfa::BitSet::Iterator vi(**l); vi(); vi++) {
				Block *v = G.at(*vi);
				if(v != bh)

					// for (w, v) ∈ E ∧ v ∉ IL(w) do
					for(Block::EdgeIter e = v->ins(); e(); e++)
						if(!IN_LOOPS(e->source())->contains(v->index())) {
							Block *w = e->source();

							// E' ← E' ∪ { (σ(w), σ(v)) }; D ← D ∪ { (w, v) }
							G.add(map[w->index()], map[v->index()], new Edge(e->flags()));
							D.add(*e);
							if(logFor(LOG_BLOCK))
								log << "\t\t\tadd edge " << map[w->index()] << " -> " << map[v->index()] << io::endl;
						}
			}

			// E' ← E' \ D
			for(Vector<Edge *>::Iter e(D); e(); e++) {
				if(logFor(LOG_BLOCK))
					log << "\t\t\tremove edge " << *e << io::endl;
				delete *e;
			}

			return true;
		}

	return false;
}

typedef elm::Vector<Pair<Block *, Block::EdgeIter> > stack_t;

#ifdef DO_DEBUG
static void displayStack(stack_t& S) {
	cerr << "[";
	bool fst = true;
	for(stack_t::Iterator v(S); v; v++) {
		if(fst)
			fst = false;
		else
			cerr << ", ";
		cerr << (*v).fst->index();
	}
	cerr << "]";
}
#endif


static inline bool mostlyIncludes(dfa::BitSet& SS, dfa::BitSet& IL, Block *w) {
	SS.add(w->index());
	bool res = SS.includes(IL);
	SS.remove(w->index());
	return res;
}


/**
 * Compute IN_LOOPS properties and computes the list
 * of loop entries.
 * @param maker	CFG to work on.
 * @param L		To store found loops.
 */
void LoopReductor::computeInLoops(CFGMaker& G, loops_t &L) {

	// D ← { ν }
	dfa::BitSet D(G.count());
	// S ← [(ν, {(ν, v) ∈ E})]
	stack_t S;
	S.add(pair(G.entry(), G.entry()->outs()));
	dfa::BitSet SS(G.count());
	SS.add(G.entry()->index());
	// L ← ∅
	L.clear();

	// for v ∈ V do IL(v) ← ∅
	for(CFG::BlockIter v = G.blocks(); v(); v++)
		IN_LOOPS(*v) = new dfa::BitSet(G.count());

	// while S ≠ [] do
	while(S) {
		DEBUG(""; displayStack(S); cerr << io::endl);

		// let (v, e) = top(S) in
		Block *v = S.top().fst;
		Block::EdgeIter &e = S.top().snd;

		// if e = ∅ then
		if(e.ended()) {
			S.pop();
			SS.remove(v->index());
			DEBUG("pop " << v->index() << io::endl);
		}

		else {

			// let (v, w)::t = e in
			Block *w = (*e)->sink();
			// top(S) ← (v, t)
			e.next();

			// if w ∉ D then
			if(!D.contains(w->index())) {

				// push(w, {(w, u) ∈ E})
				S.push(pair(w, w->outs()));
				SS.add(w->index());

				// D ← D ∪ { v }
				D.add(w->index());
				DEBUG("push " << w->index() << io::endl);
				continue;
			}

			// if w ∈ S then -- loop found
			int p;
			for(p = S.length() - 1; p >= 0 && S[p].fst != w; p--);
			if(p >= 0) {
				DEBUG("loop found at " << w->index() << io::endl);

				// if ¬∃ l ∈ L ∧ h ∈ L then
				if(!IN_LOOPS(w)->contains(w->index())) {

					// L ← L ∪ { { w } }
					dfa::BitSet *hs = new dfa::BitSet(G.count());
					hs->add(w->index());
					L.add(hs);

					// IL(w) ← IL(w) ∪ { w }
					IN_LOOPS(w)->add(w->index());
				}

				// for u  ∈ S[w, v] do IL(u) ← IL(w)
				for(int i = p; i < S.length(); i++)
					IN_LOOPS(S[i].fst)->add(**IN_LOOPS(w));
			}

			// else if IL(w) ⊆ S then -- path join
			else if(mostlyIncludes(SS, **IN_LOOPS(w), w)) {
				DEBUG("join found at " << w->index() << io::endl);

				// if ∃ h = last{u ∈ S ∧ u ∈ IL(w)} then
				int h;
				for(h = S.length() - 1; h >= 0 && !IN_LOOPS(w)->contains(S[h].fst->index()); h--);
				if(h >= 0)
					// for u ∈ S[h, w] do IL(u) ← IL[h]
					for(int u = h; u < S.length(); u++)
						IN_LOOPS(S[u].fst)->add(**IN_LOOPS(S[h].fst));
			}

			// irreducible loop
			else {
				DEBUG("IL(" << w->index() << ") = " << **IN_LOOPS(w) << io::endl);
				DEBUG("irreducible found at " << w->index() << io::endl);

				// if ∃h = last{u ∈ S ∧ u ∈ IL(w) } then
				int h;
				for(h = S.length() - 1; h >= 0 && !IN_LOOPS(w)->contains(S[h].fst->index()); h--)
					;
				DEBUG("h = " << S[h] << io::endl);
				if(h >= 0)

					// for u ∈ S[h, w] do IL(u) ← IL[h]
					for(int u = h; u < S.length(); u++)
						IN_LOOPS(S[u].fst)->add(**IN_LOOPS(S[h].fst));

				// IL(w) ← IL(w) ∪ { w }
				IN_LOOPS(w)->add(w->index());

				// let nh = IL(w) \ S in
				dfa::BitSet nh = dfa::BitSet(**IN_LOOPS(w));
				nh.remove(SS);

				// let wl = ∪{l ∈ L ∧ l ∩ nh ≠ ∅ } l ∪ { w } in
				// L ← { l ∈ L ∧ l ∩ wl = ∅ } ∪ { wl }
				dfa::BitSet *wl = new dfa::BitSet(nh);
				int j = 0;
				for(int i = 0; i < L.length(); i++) {
					if(L[i]->meets(nh)) {
						wl->add(*L[i]);
						DEBUG("merge with loop " << i << ": " << *L[i] << ": " << *wl << "\n");
						delete L[i];
					}
					else {
						DEBUG("merge with loop " << i << ": " << *L[i] << " -> no\n");
						L[j++] = L[i];
					}
				}
				L.setLength(j);
				L.add(wl);
				DEBUG("adding loop " << *wl << io::endl);
			}
		}
	}

	// closure of headers
	for(loops_t::Iter l(L); l(); l++) {
		for(dfa::BitSet::Iterator h(**l); h(); h++)
			IN_LOOPS(G.at(*h))->add(**l);
		if(logFor(LOG_BLOCK))
			log << "\t\tloop headed by " << **l << io::endl;
	}
	for(CFG::BlockIter v = G.blocks(); v(); v++) {
		for(dfa::BitSet::Iterator h(**IN_LOOPS(*v)); h(); h++)
			IN_LOOPS(*v)->add(**IN_LOOPS(G.at(*h)));
		if(logFor(LOG_BLOCK))
			log << "\t\t" << *v << " in " << **IN_LOOPS(*v) << io::endl;
	}
}


/**
 * Ensure that no more irregular loop remains in the program representation.
 */
p::feature REDUCED_LOOPS_FEATURE ("otawa::REDUCED_LOOPS_FEATURE", p::make<LoopReductor>());

} // otawa
