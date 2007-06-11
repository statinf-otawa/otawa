/*
 * $Id$
 * Copyright (c) 2005-06 IRIT-UPS
 * 
 * CATBuilder class interface.
 */
#ifndef OTAWA_IPET_CACHE_CATBUILDER_H
#define OTAWA_IPET_CACHE_CATBUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {
	
// Extern class
namespace dfa {
	class BitSet;
}
class CFG;
class ContextTree;
class LBlock;
class LBlockSet;


// Categorisation
typedef enum category_t {
	INVALID_CATEGORY = 0,
	ALWAYS_HIT = 1,
	FIRST_HIT = 2,
	FIRST_MISS = 3,
	ALWAYS_MISS = 4
} category_t;
	

// CATBuilder class
class CATBuilder: public Processor {
	dfa::BitSet *buildLBLOCKSET(LBlockSet *lcache, ContextTree *root);
	void processLBlockSet(WorkSpace *fw, LBlockSet *lbset);
	void setCATEGORISATION(LBlockSet *lineset, ContextTree *S, int dec);
	void worst(LBlock *line, ContextTree *S, LBlockSet *cacheline, int dec); 
	
public:

	static Identifier<bool> NON_CONFLICT;
	CATBuilder(void);

	// CFGProcessor overload
	virtual void processWorkSpace(WorkSpace *fw );
};

// Features
extern Feature<CATBuilder> ICACHE_CATEGORY_FEATURE;

// Properties
extern Identifier<category_t> CATEGORY;
extern Identifier<BasicBlock *> LOWERED_CATEGORY;

}	// otawa

#endif // OTAWA_IPET_CATBUILDER_H
