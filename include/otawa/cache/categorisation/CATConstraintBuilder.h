/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/prog/CATConstraintsBuilder.h -- CATConstraintsBuilder class interface.
 */
#ifndef OTAWA_CACHE_CATCONSTRAINTBUILDER_H
#define OTAWA_CACHE_CATCONSTRAINTBUILDER_H


#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/util/ContextTree.h>
#include <otawa/ilp/System.h>
#include <otawa/util/BitSet.h>

namespace otawa {
	
class LBlockSet;
class LBlock;
class CATNode;
class CATConstraintBuilder: public Processor {
	static Identifier ID_In;
	static Identifier ID_Out;
	static Identifier ID_Set;
	static Identifier ID_Cat;
	
	void processLBlockSet(FrameWork *fw, LBlockSet *lbset);
	
	
public:
	static int counter;
	// categorization
	typedef enum Categorization_t {
		INVALID = 0,
		ALWAYSHIT = 1,
		FIRSTHIT = 2,
		FIRSTMISS = 3,
		ALWAYSMISS = 4
	} Categorization_t;
	
	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
	BitSet *buildLBLOCKSET(LBlockSet *lcache, ilp::System *system,ContextTree *root);
	void setCATEGORISATION(LBlockSet *lineset , ContextTree *S , int dec);
	void worst(LBlock *line , ContextTree *S , LBlockSet *cacheline , int dec); 
};

}	// otawa


#endif //OTAWA_TEST_CATCONSTRAINTBUILDER_H_
