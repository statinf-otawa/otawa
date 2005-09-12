/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	test/ccg/CCGDFA.h -- CCGDFA class interface.
 */
#ifndef OTAWA_TEST_CCG_CCGDFA_H
#define OTAWA_TEST_CCG_CCGDFA_H

#include <elm/util/BitVector.h>
#include <otawa/util/DFA.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hardware/Cache.h>


namespace otawa {
	
	class BasicBlock;
	class LBlockSet;
class CCGDFA : public DFA {
    LBlockSet *ccggraph;
    CFG *cfglb;
    const Cache *cach;
    static int vars;
	public:
	// DFA overload
	CCGDFA (LBlockSet *point, CFG *cfg, const Cache *mem){
		ccggraph = point;
		cfglb = cfg;
		cach = mem;		
	};
	virtual DFASet *initial(void);
	virtual DFASet *generate(BasicBlock *bb);
	virtual DFASet *kill(BasicBlock *bb);
	void addCCGEDGES(CFG *cfg ,Identifier *in_id, Identifier *out_id);
	virtual void clear(DFASet *set);
	virtual void merge(DFASet *acc, DFASet *set);
};

}	// otawa

#endif	// OTAWA_TEST_CCG_CCGDFA_H
