/*
 *	$Id$
 *	IterativeDFA class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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
#ifndef OTAWA_UTIL_ITERATIVE_DFA_H
#define OTAWA_UTIL_ITERATIVE_DFA_H

#include <elm/assert.h>
#include <elm/genstruct/VectorQueue.h>
#include <elm/util/BitVector.h>
#include <otawa/cfg.h>

#ifdef OTAWA_IDFA_DEBUG
#	define OTAWA_IDFA_TRACE(x)	cerr << x << io::endl
#	define OTAWA_IDFA_DUMP(m, s)		{ cerr << m << "= "; prob.dump(cerr, s); cerr << io::endl; }
#else
#	define OTAWA_IDFA_TRACE(x)
#	define OTAWA_IDFA_DUMP(m, s)
#endif

namespace otawa { namespace dfa {

// predeclaration
class Successor;

// Predecessor
class Predecessor: public PreIterator<Predecessor, BasicBlock *> {
	BasicBlock::InIterator iter;
	inline void look(void) {
		while(iter && iter->kind() == Edge::CALL)
			iter++;
	}
public:
	typedef Successor Forward;
	inline Predecessor(BasicBlock *bb): iter(bb) {
		look();
	};
	static inline BasicBlock *entry(CFG& cfg) { return cfg.entry(); }
	inline BasicBlock *item(void) const { return iter.item()->source(); };
	inline bool ended(void) const { return iter.ended(); };
	inline void next(void) { iter.next(); look(); };
};


// Successor
class Successor: public PreIterator<Successor, BasicBlock *> {
	BasicBlock::OutIterator iter;
	inline void look(void) {
		while(iter && iter->kind() == Edge::CALL)
			iter++;
	}
public:
	typedef Predecessor Forward;
	inline Successor(BasicBlock *bb): iter(bb) {
		look();
	};
	static inline BasicBlock *entry(CFG& cfg) { return cfg.exit(); }
	inline BasicBlock *item(void) const { return iter.item()->target(); };
	inline bool ended(void) const { return iter.ended(); };
	inline void next(void) { iter.next(); look(); };
};


// DFAEngine class
template <class Problem, class Set, class Iter = Predecessor>
class IterativeDFA {
	Problem& prob;
	CFG& _cfg;
	int cnt;
	Set **ins, **outs, **gens, **kills;
public:
	inline IterativeDFA(Problem& problem, CFG& cfg);
	inline ~IterativeDFA(void);
	inline void compute(void);
	inline Set *inSet(BasicBlock *bb);
	inline Set *outSet(BasicBlock *bb);
	inline Set *genSet(BasicBlock *bb);
	inline Set *killSet(BasicBlock *bb);
};


// IterativeDFA::IterativeDFA inline
template <class Problem, class Set, class Iter>
inline IterativeDFA<Problem, Set, Iter>::IterativeDFA(Problem& problem, CFG& cfg)
: prob(problem), _cfg(cfg), cnt(cfg.countBB()) {
	ins = new Set *[cnt];
	outs = new Set *[cnt];
	gens = new Set *[cnt];
	kills = new Set *[cnt];
	for(CFG::BBIterator bb(&_cfg); bb; bb++) {
		int idx = INDEX(bb);
		ins[idx] = prob.empty();
		outs[idx] = prob.empty();
		gens[idx] = prob.gen(bb);
		kills[idx] = prob.kill(bb);
	}
}


// IterativeDFA::~IterativeDFA() inline
template <class Problem, class Set, class Iter>
inline IterativeDFA<Problem, Set, Iter>::~IterativeDFA(void) {
	for(int i = 0; i < cnt; i++) {
		if(ins[i])
			prob.free(ins[i]);
		if(outs[i])
			prob.free(outs[i]);
		prob.free(gens[i]);
		prob.free(kills[i]);
	}
	delete [] ins;
	delete [] outs;
	delete [] gens;
	delete [] kills;
}


// IterativeDFA::inSet() inline
template <class Problem, class Set, class Iter>
inline Set *IterativeDFA<Problem, Set, Iter>::inSet(BasicBlock *bb) {
	return ins[INDEX(bb)];
}


// IterativeDFA::outSet() inline
template <class Problem, class Set, class Iter>
inline Set *IterativeDFA<Problem, Set, Iter>::outSet(BasicBlock *bb) {
	return outs[INDEX(bb)];
}


// IterativeDFA::genSet() inline
template <class Problem, class Set, class Iter>
inline Set *IterativeDFA<Problem, Set, Iter>::genSet(BasicBlock *bb) {
	return gens[INDEX(bb)];
}


// IterativeDFA::killSet() inline
template <class Problem, class Set, class Iter>
inline Set *IterativeDFA<Problem, Set, Iter>::killSet(BasicBlock *bb) {
	return kills[INDEX(bb)];
}


// IterativeDFA::compute() inline
template <class Problem, class Set, class Iter>
inline void IterativeDFA<Problem, Set, Iter>::compute(void) {

	// initialization
	VectorQueue<BasicBlock *> todo;
	BitVector present(_cfg.countBB());
	Set *comp = prob.empty(), *ex;
	for(CFG::BBIterator bb(&_cfg); bb; bb++) {
			OTAWA_IDFA_TRACE("DFA: push BB" << bb->number());
			todo.put(bb);
			present.set(bb->number());
		}

	// perform until no change
	while(todo) {
		BasicBlock *bb = todo.get();
		int idx = bb->number();
		ASSERT(idx >= 0);
		present.clear(idx);
		OTAWA_IDFA_TRACE("DFA: processing BB" << idx);

		// IN = union OUT of predecessors
		prob.reset(ins[idx]);
		for(Iter pred(bb); pred; pred++) {
			BasicBlock *bb_pred = pred;
			int pred_idx = bb_pred->number();
			ASSERT(pred_idx >= 0);
			prob.merge(ins[idx], outs[pred_idx]);
		}

		// OUT = IN \ KILL U GEN
		prob.set(comp, ins[idx]);
		OTAWA_IDFA_DUMP("IN", comp);
		prob.diff(comp, kills[idx]);
		OTAWA_IDFA_DUMP("KILL", kills[idx]);
		prob.add(comp, gens[idx]);
		OTAWA_IDFA_DUMP("GEN", gens[idx]);
		OTAWA_IDFA_DUMP("OUT", comp);

		// Any modification ?
		if(!prob.equals(comp, outs[idx])) {

			// record new out
			ex = outs[idx];
			outs[idx] = comp;
			comp = ex;

			// add successors
			for(typename Iter::Forward next(bb); next; next++)
				if(!present.bit(next->number())) {
					OTAWA_IDFA_TRACE("DFA: push BB" << next->number());
					todo.put(next);
					present.set(next->number());
				}
		}
		prob.reset(comp);
	}

	// cleanup
	prob.free(comp);
}

} } // otawa::dfa

#endif // OTAWA_UTIL_ITERATIVE_DFA_H
