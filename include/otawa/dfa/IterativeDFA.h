/*
 * $Id$
 * Copyright (c) 2007 IRIT-UPS
 *
 * IterativeDFA class interface.
 */
#ifndef OTAWA_UTIL_ITERATIVE_DFA_H
#define OTAWA_UTIL_ITERATIVE_DFA_H

#include <assert.h>
#include <otawa/cfg.h>

namespace otawa { namespace dfa {

// Predecessor
class Predecessor: public PreIterator<Predecessor, BasicBlock *> {
	BasicBlock::InIterator iter;
	inline void look(void) {
		while(iter && iter->kind() == Edge::CALL)
			iter++;
	}
public:
	inline Predecessor(BasicBlock *bb): iter(bb) {
		look();
	};
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
	inline Successor(BasicBlock *bb): iter(bb) {
		look();
	};
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
			delete ins[i];
		if(outs[i])
			delete outs[i];
		delete gens[i];
		delete kills[i];
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
	bool changed = true;
	Set *comp = prob.empty(), *ex;
	while(changed) {
		changed = false;
		for(CFG::BBIterator bb(&_cfg); bb; bb++) {
			int idx = bb->number();
			assert(idx >= 0);
			
			// IN = union OUT of predecessors
			prob.reset(ins[idx]);
			for(Iter pred(bb); pred; pred++) {
				BasicBlock *bb_pred = pred;
				int pred_idx = bb_pred->number();
				assert(pred_idx >= 0);
				prob.merge(ins[idx], outs[pred_idx]);
			}
			
			// OUT = IN \ KILL U GEN
			prob.set(comp, ins[idx]);
			prob.diff(comp, kills[idx]);
			prob.add(comp, gens[idx]);
			
			// Any modification ?
			if(!prob.equals(comp, outs[idx])) {
				ex = outs[idx];
				outs[idx] = comp;
				comp = ex;
				changed = true;
			}
			prob.reset(comp);
		}
	}
}

} } // otawa::dfa

#endif // OTAWA_UTIL_ITERATIVE_DFA_H
