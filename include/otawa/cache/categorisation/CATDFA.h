#ifndef _CATDFA_H_
#define _CATDFA_H_

#include <elm/util/BitVector.h>
#include <otawa/util/DFA.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>


namespace otawa {
	
	class BasicBlock;
	class LBlockSet;
class CATDFA : public DFA {
    LBlockSet *lines;
    CFG *cfglb;
    const hard::Cache *cach;
    
	public:
	// DFA overload
	CATDFA (LBlockSet *point, CFG *cfg, const hard::Cache *mem){
		lines = point;
		cfglb = cfg;
		cach = mem;	
	};
	virtual DFASet *initial(void);
	virtual DFASet *generate(BasicBlock *bb);
	virtual DFASet *kill(BasicBlock *bb);
	virtual void clear(DFASet *set);
	virtual void merge(DFASet *acc, DFASet *set);
};

}	// otawa

#endif //_CATDFA_H_
