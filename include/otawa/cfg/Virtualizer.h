#ifndef CFG_VIRTUALIZER_H_
#define CFG_VIRTUALIZER_H_

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {



class Virtualizer: public Processor {

	public:
	Virtualizer(void);
	virtual void processWorkSpace(WorkSpace*);
	virtual void configure(const PropList& props);
	
	
	private:	
	void virtualize(struct call_t*, CFG *cfg, VirtualCFG *vcfg, BasicBlock *entry, BasicBlock *exit);
	bool isInlined();
	bool virtual_inlining;
	CFG *entry;
	elm::genstruct::HashTable<void *, VirtualCFG *> cfgMap;
};

extern Feature<Virtualizer> VIRTUALIZED_CFG_FEATURE;
extern Identifier<bool> VIRTUAL_INLINING;

}


#endif 
