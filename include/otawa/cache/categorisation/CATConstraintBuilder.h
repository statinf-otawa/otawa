/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/test/CATConstraintsBuilder.h -- CATConstraintsBuilder class interface.
 */

#ifndef OTAWA_TEST_CATCONSTRAINTBUILDER_H_
#define OTAWA_TEST_CATCONSTRAINTBUILDER_H_


#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/util/ContextTree.h>
#include <otawa/ilp/System.h>
#include <otawa/util/DFABitSet.h>

namespace otawa {
	
class LBlockSet;
class LBlock;
class CATNode;
class CATConstraintBuilder: public CFGProcessor {
	FrameWork *fw;
	static Identifier ID_In;
	static Identifier ID_Out;
	static Identifier ID_Set;
	static Identifier ID_Cat;
	
	
	
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
	inline CATConstraintBuilder(FrameWork *framework);
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
	DFABitSet *buildLBLOCKSET(LBlockSet *lcache, ilp::System *system,ContextTree *root);
	void setCATEGORISATION(LBlockSet *lineset , ContextTree *S , int dec);
	void worst(LBlock *line , ContextTree *S , LBlockSet *cacheline , int dec); 
};
inline CATConstraintBuilder::CATConstraintBuilder(FrameWork *framework)
 : fw(framework) {
		assert(fw);
}

}	// otawa


#endif //OTAWA_TEST_CATCONSTRAINTBUILDER_H_
