/*
 *	BlockAbsInt class interface and implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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
#ifndef OTAWA_AI_BLOCKABSINT_H
#define OTAWA_AI_BLOCKABSINT_H

#include <elm/array.h>
#include <elm/data/SortedList.h>
#include <elm/util/BitVector.h>
#include <otawa/ai/DefaultBlockStore.h>
#include <otawa/ai/features.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>

#ifdef OTAWA_AI_DEBUG
#	define OTAWA_AI_PRINT(x)	cerr << x << io::endl
#else
#	define OTAWA_AI_PRINT(x)
#endif

namespace otawa { namespace ai {

template <class D, class S = DefaultBlockStore<typename D::t> >
class BlockAnalysis: public CFGProcessor {
public:
	static p::declare reg;
	BlockAnalysis(p::declare& r): CFGProcessor(r) { }

protected:

	typedef typename D::t t;

	inline D& domain() { return d; }

	inline t out(Block *v) const { return store.get(v); }

	t in(Block *v) const {
		t x = d.bot();
		for(auto e: v->inEdges())
			d.set(x, d.join(x, out(e->source())));
		return x;
	}

	void setup(WorkSpace *ws) {
		d.setup(ws);
	}

	void cleanup(WorkSpace *ws) {
		d.cleanup(ws);
	}

	void processWorkSpace(WorkSpace *ws) override {
		const CFGCollection& cfgs = *otawa::COLLECTED_CFG_FEATURE.get(ws);
		store.init(cfgs, d.bot());
		todo.comparator().r = ai::CFG_RANKING_FEATURE.get(ws);
		intodo = BitVector(cfgs.countBlocks());
		typename D::t x;

		// prepare to-do list
		Block *v = cfgs.entry()->entry();
		store.set(v, d.init(ws));
		OTAWA_AI_PRINT("initial = " << io::p(d.init(ws), d));
		for(auto e: v->outEdges()) {
			todo.add(e->sink());
			intodo.set(e->sink()->id());
		}

		// repeat until fix-point
		while(todo) {

			// get the next vertex to process
			v = get();
			OTAWA_AI_PRINT("processing " << v << " of " << v->cfg());

			// join predecessors
			d.set(x, d.bot());
			for(auto e: v->inEdges()) {
				OTAWA_AI_PRINT("\tfrom " << e->source() << ": " << io::p(store.get(e->source()), d));
				x = d.join(x, store.get(e->source()));
			}
			OTAWA_AI_PRINT("\tIN = " << io::p(x, d));

			// update for basic block
			if(v->isBasic()) {
				d.set(x, d.update(v, x));
				OTAWA_AI_PRINT("\tOUT = " << io::p(x, d));
				if(!d.equals(x, store.get(v))) {
					store.set(v, x);
					for(auto e: v->outEdges())
						put(e->sink());
				}
			}

			// update for synthetic block
			else if(v->isSynth()) {
				SynthBlock *c = v->toSynth();
				if(c->callee() == nullptr) {
					store.set(v, x);
					for(auto e: v->outEdges())
						put(e->sink());
				}
				else {
					Block *w = c->callee()->entry();
					if(!d.equals(x, out(w))) {
						store.set(w, x);
						for(auto e: w->outEdges())
							put(e->sink());
					}
				}
			}

			// update for exit
			else if(v->isExit())
				for(auto c: v->cfg()->callers())
					if(!d.equals(x, out(c))) {
						store.set(c, x);
						for(auto e: c->outEdges())
							put(e->sink());
					}

		}

	}

	///
	void processCFG(WorkSpace *ws, CFG *cfg) override {
	}

private:

	inline void put(Block *v) {
		if(!intodo.bit(v->id())) {
			todo.add(v);
			intodo.set(v->id());
		}
	}

	inline Block *get() {
		Block *v = todo.first();
		todo.removeFirst();
		intodo.clear(v->id());
		return v;
	}

	class Comparator {
	public:
		typedef Block *t;
		int doCompare(Block *v, Block *w) const {
			return r->rankOf(v) - r->rankOf(w);
		}
		ai::CFGRanking *r;
	};

	SortedList<Block *, Comparator> todo;
	BitVector intodo;
	S store;
	D d;
};

template <class D, class S> p::declare BlockAnalysis<D, S>::reg = p::init("", Version(1, 0, 0))
	.extend<CFGProcessor>()
	.require(ai::CFG_RANKING_FEATURE);

} }	// otawa::ai

#endif	// OTAWA_AI_BLOCKABSINT_H
