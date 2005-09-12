#ifndef _CCGCONSTRAINTBUILDER_H_
#define _CCGCONSTRAINTBUILDER_H_

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/util/ContextTree.h>
#include <otawa/hardware/Cache.h>



namespace otawa {
class LBlockSet;
class CFG;
class LBlock;
class CCGNode;
class CCGConstraintBuilder: public CFGProcessor {
	FrameWork *fw;
	
	
public:
	inline CCGConstraintBuilder(FrameWork *framework);
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
	void addConstraintHEADER (CFG *cfg,LBlockSet *graph, ContextTree *ct, LBlock *boc);
};
inline CCGConstraintBuilder::CCGConstraintBuilder(FrameWork *framework)
 : fw(framework) {
		assert(fw);
}

}	// otawa


#endif //_CCGCONSTRAINTBUILDER_H_
