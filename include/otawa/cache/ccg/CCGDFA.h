/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/ccg/CCGDFA.h -- CCGDFA class interface.
 */
#ifndef OTAWA_TEST_CCG_CCGDFA_H
#define OTAWA_TEST_CCG_CCGDFA_H

#include <otawa/util/DFAEngine.h>
#include <otawa/util/BitSet.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>
#include <otawa/cfg.h>


namespace otawa {

// Extern classes	
class BasicBlock;
class LBlockSet;

class CCGDomain: public BitSet {

public:
	inline CCGDomain(int size) : BitSet(size) {
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
		return(BitSet::equals(*d));
	}
};

// CCGProblem Class
class CCGProblem {
    LBlockSet *ccggraph;
    const hard::Cache *cach;
    static int vars;
    int size;
    FrameWork *fw;

public:

	typedef CCGDomain domain_t;
	inline CCGProblem (LBlockSet *_ccggraph, int _size , const hard::Cache *_cach, FrameWork *_fw);

	CCGDomain *empty(void) {
		CCGDomain *tmp = new CCGDomain(size);
		tmp->reset();
		return(tmp);
	}
	
	CCGDomain *gen(CFG *cfg, BasicBlock *bb);
	CCGDomain *preserve(CFG *cfg, BasicBlock *bb);

	void free(CCGDomain *d) {
		delete d;
	}
};


// Inlines
inline CCGProblem::CCGProblem (LBlockSet *_ccggraph, int _size, const hard::Cache *_cach, FrameWork *_fw) {
	ccggraph = _ccggraph;
	cach = _cach;
	size = _size;
	fw = _fw;
}

}	// otawa

#endif	// OTAWA_TEST_CCG_CCGDFA_H
