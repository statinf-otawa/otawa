/*
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
#include <elm/data/VectorQueue.h>
#include <elm/util/BitVector.h>
#include <otawa/graph/DiGraph.h>

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
class Predecessor: public PreIterator<Predecessor, graph::Vertex *> {
public:
	typedef Successor Forward;
	inline Predecessor(graph::Vertex *v): iter(v->ins()) { }
	inline graph::Vertex *item(void) const { return iter.item()->source(); }
	inline bool ended(void) const { return iter.ended(); }
	inline void next(void) { iter.next(); }
private:
	typename graph::Vertex::EdgeIter iter;
};


// Successor
class Successor: public PreIterator<Successor, graph::Vertex *> {
public:
	typedef Predecessor Forward;
	inline Successor(graph::Vertex *v): iter(v->outs()) { }
	inline graph::Vertex *item(void) const { return iter.item()->sink(); }
	inline bool ended(void) const { return iter.ended(); }
	inline void next(void) { iter.next(); }
private:
	graph::Vertex::EdgeIter iter;
};


// DFAEngine class
template <class Problem, class Set, class G, class Iter = Predecessor>
class IterativeDFA {
public:
	inline IterativeDFA(Problem& problem, G *g, typename G::vertex_t *entry);
	inline ~IterativeDFA(void);
	inline void compute(void);
	inline Set *inSet(typename G::vertex_t *bb);
	inline Set *outSet(typename G::vertex_t *bb);
	inline Set *genSet(typename G::vertex_t *bb);
	inline Set *killSet(typename G::vertex_t *bb);

private:
	Problem& prob;
	G *_g;
	int cnt;
	Set **ins, **outs, **gens, **kills;
	typename G::vertex_t *_entry;
};


// IterativeDFA::IterativeDFA inline
template <class Problem, class Set, class G, class Iter>
inline IterativeDFA<Problem, Set, G, Iter>::IterativeDFA(Problem& problem, G *g, typename G::vertex_t *entry)
: prob(problem), _g(g), cnt(g->count()), _entry(entry) {
	ins = new Set *[cnt];
	outs = new Set *[cnt];
	gens = new Set *[cnt];
	kills = new Set *[cnt];
	for(auto v: *_g) {
		int idx = v->index();
		ins[idx] = prob.empty();
		outs[idx] = prob.empty();
		gens[idx] = prob.gen(v);
		kills[idx] = prob.kill(v);
	}
}


// IterativeDFA::~IterativeDFA() inline
template <class Problem, class Set, class G, class Iter>
inline IterativeDFA<Problem, Set, G, Iter>::~IterativeDFA(void) {
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
template <class Problem, class Set, class G, class Iter>
inline Set *IterativeDFA<Problem, Set, G, Iter>::inSet(typename G::vertex_t *bb) {
	return ins[bb->index()];
}


// IterativeDFA::outSet() inline
template <class Problem, class Set, class G, class Iter>
inline Set *IterativeDFA<Problem, Set, G, Iter>::outSet(typename G::vertex_t *bb) {
	return outs[bb->index()];
}


// IterativeDFA::genSet() inline
template <class Problem, class Set, class G, class Iter>
inline Set *IterativeDFA<Problem, Set, G, Iter>::genSet(typename G::vertex_t *bb) {
	return gens[bb->index()];
}


// IterativeDFA::killSet() inline
template <class Problem, class Set, class G, class Iter>
inline Set *IterativeDFA<Problem, Set, G, Iter>::killSet(typename G::vertex_t *bb) {
	return kills[bb->index()];
}


// IterativeDFA::compute() inline
template <class Problem, class Set, class G, class Iter>
inline void IterativeDFA<Problem, Set, G, Iter>::compute(void) {

	// initialization
	VectorQueue<typename G::vertex_t *> todo;
	BitVector present(_g->count());
	Set *comp = prob.empty(), *ex;
	for(auto bb: *_g) {
			OTAWA_IDFA_TRACE("DFA: push BB" << bb->index());
			todo.put(bb);
			present.set(bb->index());
		}

	// perform until no change
	while(todo) {
		typename G::vertex_t *bb = todo.get();
		int idx = bb->index();
		ASSERT(idx >= 0);
		present.clear(idx);
		OTAWA_IDFA_TRACE("DFA: processing BB" << idx);

		// IN = union OUT of predecessors
		prob.reset(ins[idx]);
		for(Iter pred(bb); pred(); pred++) {
			typename G::vertex_t *bb_pred = static_cast<typename G::vertex_t *>(*pred);
			int pred_idx = bb_pred->index();
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
			for(typename Iter::Forward next(bb); next(); next++)
				if(!present.bit(next->index())) {
					OTAWA_IDFA_TRACE("DFA: push BB" << next->index());
					todo.put(static_cast<typename G::vertex_t *>(*next));
					present.set(next->index());
				}
		}
		prob.reset(comp);
	}

	// cleanup
	prob.free(comp);
}

} } // otawa::dfa

#endif // OTAWA_UTIL_ITERATIVE_DFA_H
