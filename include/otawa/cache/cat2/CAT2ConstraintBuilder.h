#ifndef CACHE_CAT2CONSTRAINTBUILDER_H_
#define CACHE_CAT2CONSTRAINTBUILDER_H_

#include <otawa/proc/Processor.h>

namespace otawa {
	
extern Identifier<ilp::Var *> HIT_VAR;
extern Identifier<ilp::Var *> MISS_VAR;

class CAT2ConstraintBuilder : public otawa::Processor {
	bool _explicit;
	public:
	CAT2ConstraintBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList& props);
	virtual void setup(otawa::WorkSpace*);
};

}

#endif /*CACHE_CAT2CONSTRAINTBUILDER_H_*/
