#ifndef _CATDFA_H_
#define _CATDFA_H_

#include <otawa/dfa/BitSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>


namespace otawa {
	
	class BasicBlock;
	class LBlockSet;



class CATDomain: public dfa::BitSet {

public:
	inline CATDomain(int size) : dfa::BitSet(size) {
	}
	void reset(void) {
		empty();
	}
	void join(CATDomain *d) {
		add(*d);
	}
	void meet(CATDomain *d) {
		mask(*d);
	}
	bool equals(CATDomain *d) {
		return(BitSet::equals(*d));
	}
};


class CATProblem {
    LBlockSet *lines;
    const hard::Cache *cach;
    WorkSpace *fw;
    int size;
    static int vars;
    
        public:

	typedef CATDomain domain_t;

	CATDomain *empty(void) {
		CATDomain *tmp = new CATDomain(size);
		tmp->reset();
		return(tmp);
	}
	
	CATDomain *gen(CFG *cfg, BasicBlock *bb);
	CATDomain *preserve(CFG *cfg, BasicBlock *bb);

	void free(CATDomain *d) {
		delete d;
	}
	
	inline  void edge(CATDomain *target, CFG *cfg1, BasicBlock *bb1, CFG *cfg2, BasicBlock *bb2) {
	}
    
	CATProblem (LBlockSet *point, int _size, const hard::Cache *mem, WorkSpace *_fw){
		lines = point;
		size = _size;
		cach = mem;	
		fw = _fw;
	};
	
};

}	// otawa

#endif //_CATDFA_H_
