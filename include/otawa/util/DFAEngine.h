/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/otawa/util/DFAEngine.h -- DFAEngine class interface.
 */
#ifndef OTAWA_UTIL_DFAENGINE_H
#define OTAWA_UTIL_DFAENGINE_H

#include <assert.h>
#include <otawa/cfg.h>
#include <otawa/util/DFABitSet.h>

namespace otawa { namespace util {

// DFAPredecessor
class DFAPredecessor: public PreIterator<DFAPredecessor, BasicBlock *> {
	BasicBlock::InIterator iter;
	inline void look(void) {
		while(iter && iter->kind() == Edge::CALL)
			iter++;
	}
public:
	inline DFAPredecessor(BasicBlock *bb): iter(bb) {
		look();
	};
	inline BasicBlock *item(void) const { return iter.item()->source(); };
	inline bool ended(void) const { return iter.ended(); };
	inline void next(void) { iter.next(); look(); };
};


// DFASuccessor
class DFASuccessor: public PreIterator<DFASuccessor, BasicBlock *> {
	BasicBlock::OutIterator iter;
	inline void look(void) {
		while(iter && iter->kind() == Edge::CALL)
			iter++;
	}
public:
	inline DFASuccessor(BasicBlock *bb): iter(bb) {
		look();
	};
	inline BasicBlock *item(void) const { return iter.item()->target(); };
	inline bool ended(void) const { return iter.ended(); };
	inline void next(void) { iter.next(); look(); };
};


// DFAEngine class
template <class Problem, class Set, class Iter = DFAPredecessor>
class DFAEngine {
	Problem& prob;
	CFG& _cfg;
	int cnt;
	Set **ins, **outs, **gens, **kills;
public:
	inline DFAEngine(Problem& problem, CFG& cfg);
	inline ~DFAEngine(void);
	inline void compute(void);
	inline Set *inSet(BasicBlock *bb);
	inline Set *outSet(BasicBlock *bb); 
	inline Set *genSet(BasicBlock *bb);
	inline Set *killSet(BasicBlock *bb); 
};


// DFAEngine::DFAEngine inline
template <class Problem, class Set, class Iter>
inline DFAEngine<Problem, Set, Iter>::DFAEngine(Problem& problem, CFG& cfg)
: _cfg(cfg), cnt(cfg.countBB()), prob(problem) {
	ins = new Set *[cnt];
	outs = new Set *[cnt];
	gens = new Set *[cnt];
	kills = new Set *[cnt];
	for(elm::Iterator<BasicBlock *> bb(_cfg.bbs()); bb; bb++) {
		int idx = bb->use<int>(BasicBlock::ID_Index);
		ins[idx] = prob.empty();
		outs[idx] = prob.empty();
		gens[idx] = prob.gen(bb);
		kills[idx] = prob.kill(bb);
	}
}


// DFAEngine::~DFAEngine() inline
template <class Problem, class Set, class Iter>
inline DFAEngine<Problem, Set, Iter>::~DFAEngine(void) {
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


// DFAEngine::inSet() inline
template <class Problem, class Set, class Iter>
inline Set *DFAEngine<Problem, Set, Iter>::inSet(BasicBlock *bb) {
	return ins[bb->use<int>(BasicBlock::ID_Index)];
}


// DFAEngine::outSet() inline
template <class Problem, class Set, class Iter>
inline Set *DFAEngine<Problem, Set, Iter>::outSet(BasicBlock *bb) {
	return outs[bb->use<int>(BasicBlock::ID_Index)];
}


// DFAEngine::genSet() inline
template <class Problem, class Set, class Iter>
inline Set *DFAEngine<Problem, Set, Iter>::genSet(BasicBlock *bb) {
	return gens[bb->use<int>(BasicBlock::ID_Index)];
}


// DFAEngine::killSet() inline
template <class Problem, class Set, class Iter>
inline Set *DFAEngine<Problem, Set, Iter>::killSet(BasicBlock *bb) {
	return kills[bb->use<int>(BasicBlock::ID_Index)];
}


// DFAEngine::compute() inline
template <class Problem, class Set, class Iter>
inline void DFAEngine<Problem, Set, Iter>::compute(void) {
	bool changed = true;
	Set *comp = prob.empty(), *ex;
	while(changed) {
		changed = false;
		for(elm::Iterator<BasicBlock *> bb(_cfg.bbs()); bb; bb++) {
			int idx = bb->number();
			
			// IN = union OUT of predecessors
			ins[idx]->reset();
			for(Iter pred(bb); pred; pred++)
				if(pred) {
					BasicBlock *bb_pred = pred;
					int pred_idx = bb_pred->number();
					ins[idx]->add(outs[pred_idx]);
				}
			
			// OUT = IN \ KILL U GEN
			comp->add(ins[idx]);
			comp->remove(kills[idx]);
			comp->add(gens[idx]);
			
			// Any modification ?
			if(!comp->equals(outs[idx])) {
				ex = outs[idx];
				outs[idx] = comp;
				comp = ex;
				changed = true;
			}
			comp->empty();
		}
	}
}

} } // otawa::util

#endif // OTAWA_UTIL_DFAENGINE_H

