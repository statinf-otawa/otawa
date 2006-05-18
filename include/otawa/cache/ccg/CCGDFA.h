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
#include <otawa/hard/Cache.h>


namespace otawa {

// Extern classes	
class BasicBlock;
class LBlockSet;

// CCGDFA Class
class CCGDFA : public DFA {
    LBlockSet *ccggraph;
    CFG *cfglb;
    const hard::Cache *cach;
    static int vars;

public:
	inline CCGDFA (LBlockSet *point, CFG *cfg, const hard::Cache *mem);

	// DFA overload
	virtual DFASet *initial(void);
	virtual DFASet *generate(BasicBlock *bb);
	virtual DFASet *kill(BasicBlock *bb);
	virtual void clear(DFASet *set);
	virtual void merge(DFASet *acc, DFASet *set);
};

// Inlines
inline CCGDFA::CCGDFA (LBlockSet *point, CFG *cfg, const hard::Cache *mem) {
	ccggraph = point;
	cfglb = cfg;
	cach = mem;		
}

}	// otawa

#endif	// OTAWA_TEST_CCG_CCGDFA_H
