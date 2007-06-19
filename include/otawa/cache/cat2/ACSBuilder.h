#ifndef CACHE_ACSBUILDER_H_
#define CACHE_ACSBUILDER_H_

#include <otawa/proc/Processor.h>
#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/PERSProblem.h>

namespace otawa {
	
using namespace elm;


typedef enum fmlevel_t {
		FML_INNER = 0,
		FML_OUTER = 1,
		FML_MULTI = 2,
		FML_NONE
} fmlevel_t;
	
extern Identifier<genstruct::Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST;
extern Identifier<genstruct::Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS;
extern Identifier<fmlevel_t> FIRSTMISS_LEVEL;
extern Identifier<bool> PSEUDO_UNROLLING;
extern Identifier<MUSTProblem::Domain*> CACHE_ACS_MUST_ENTRY;

class ACSBuilder : public otawa::Processor {

	void processLBlockSet(otawa::WorkSpace*, LBlockSet *, const hard::Cache *);	
	fmlevel_t level;
	
	bool unrolling;
	MUSTProblem::Domain *must_entry;
	
	public:
	ACSBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);
	
	
	
};

extern Feature<ACSBuilder> ICACHE_ACS_FEATURE;

}

#endif /*CACHE_ACSBUILDER_H_*/
