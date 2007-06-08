#ifndef CACHE_FIRSTLASTBUILDER_H_
#define CACHE_FIRSTLASTBUILDER_H_

#include <otawa/proc/Processor.h>


namespace otawa {
	
using namespace elm;

	
extern Identifier<LBlock**> LAST_LBLOCK;

extern Identifier<bool> LBLOCK_ISFIRST;

class FirstLastBuilder : public otawa::CFGProcessor {

	void processLBlockSet(CFG *, LBlockSet *, const hard::Cache *);
	
	public:
	FirstLastBuilder(void);
	virtual void processCFG(otawa::WorkSpace*, otawa::CFG*);
	
	
};

extern Feature<FirstLastBuilder> ICACHE_FIRSTLAST_FEATURE;

}

#endif /*CACHE_FIRSTLASTBUILDER_H_*/
