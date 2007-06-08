#ifndef CACHE_CAT2BUILDER_H_
#define CACHE_CAT2BUILDER_H_

#include <otawa/proc/Processor.h>
#include <otawa/cache/cat2/ACSBuilder.h>
namespace otawa {



extern Identifier<category_t> CATEGORY;
extern Identifier<BasicBlock*> CATEGORY_HEADER;

class CAT2Builder: public CFGProcessor{
	void processLBlockSet(otawa::CFG*, LBlockSet *, const hard::Cache *);
	fmlevel_t firstmiss_level;
	public:
	CAT2Builder(void);
	virtual void processCFG(WorkSpace*, otawa::CFG*);
	virtual void setup(WorkSpace*);
	virtual void configure(const PropList &props);	
};


}


#endif /*CACHE_CAT2BUILDER_H_*/
