/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/ccg/CCGDFA.h -- CCGDFA class interface.
 */
#ifndef OTAWA_TEST_CCG_CCGDFA_H
#define OTAWA_TEST_CCG_CCGDFA_H

#include <otawa/dfa/BitSet.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg.h>


namespace otawa {

// Extern classes	
class BasicBlock;

class CCGDomain: public dfa::BitSet {

public:
	inline CCGDomain(int size) : dfa::BitSet(size) {
	}
	
	void reset(void) {
		empty();
	}
	void join(CCGDomain *d) {
		add(*d);
	}
	void meet(CCGDomain *d) {
		mask(*d);
	}
	bool equals(CCGDomain *d) {
		return(dfa::BitSet::equals(*d));
	}
};

// CCGProblem Class
class CCGProblem {
    LBlockSet *ccggraph;
    const hard::Cache *cach;
    static int vars;
    int size;
    WorkSpace *fw;
    HashTable<Pair<BasicBlock*, BasicBlock*>, Pair<dfa::BitSet*, int> > *confmap;

public:

	typedef CCGDomain domain_t;
	
	HashTable<int, dfa::BitSet*> fakePreds;
	inline CCGProblem (LBlockSet *_ccggraph, int _size , const hard::Cache *_cach, WorkSpace *_fw, HashTable<Pair<BasicBlock*, BasicBlock*>, Pair<dfa::BitSet*, int> > *_confmap);

	CCGDomain *empty(void) {
		CCGDomain *tmp = new CCGDomain(size);
		tmp->reset();
		return(tmp);
	}
	
	CCGDomain *gen(CFG *cfg, BasicBlock *bb);
	CCGDomain *preserve(CFG *cfg, BasicBlock *bb);

	inline void edge(CCGDomain *target, CFG *cfg1, BasicBlock *bb1, CFG *cfg2, BasicBlock *bb2) {

		if ((confmap != NULL) && confmap->hasKey(pair(bb2, bb1))) {

			Pair<dfa::BitSet*, int> val = confmap->get(pair(bb2, bb1)).value();
			if (fakePreds.get(val.snd, NULL) == NULL) {
				fakePreds.put(val.snd, new dfa::BitSet(*target));
			} else {
				*fakePreds.get(val.snd, NULL) = *target;
			}
			target->empty();
			target->add(size - (val.snd + 2));
			
		}
		
	}
	void free(CCGDomain *d) {
		delete d;
	}
};


// Inlines
inline CCGProblem::CCGProblem (LBlockSet *_ccggraph, int _size, const hard::Cache *_cach, WorkSpace *_fw, HashTable<Pair<BasicBlock*, BasicBlock*>, Pair<dfa::BitSet*, int> > *_confmap) 
: confmap(_confmap) {
	ccggraph = _ccggraph;
	cach = _cach;
	size = _size;
	fw = _fw;
}

}	// otawa

#endif	// OTAWA_TEST_CCG_CCGDFA_H
