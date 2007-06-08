#ifndef CFG_LOOPUNROLLER_H_
#define CFG_LOOPUNROLLER_H_

#include <otawa/proc/Processor.h>

namespace otawa {



class LoopUnroller: public Processor {

	public:
	LoopUnroller(void);
	virtual void processWorkSpace(WorkSpace*);
	
	
	private:	
	elm::genstruct::HashTable<void *, VirtualBasicBlock *> map;		
	void unroll(otawa::CFG *cfg, BasicBlock *header, VirtualCFG *vcfg);
	CFGCollection *coll;
	int idx;
};

extern Feature<LoopUnroller> UNROLLED_LOOPS_FEATURE;

}


#endif 
