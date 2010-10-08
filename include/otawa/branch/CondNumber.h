#ifndef BRANCH_CONDNUMBER_H_
#define BRANCH_CONDNUMBER_H_

#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/BasicBlock.h>
namespace otawa {
	
using namespace elm;

extern Identifier<int> COND_NUMBER;
extern Identifier<int*> COND_MAX;
extern SilentFeature NUMBERED_CONDITIONS_FEATURE;

class CondNumber : public otawa::BBProcessor {

	public:
	CondNumber(void);
	virtual void processBB(otawa::WorkSpace*, CFG *cfg, BasicBlock *bb);
	virtual void configure(const PropList &props);
	
	protected:
	
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);	
	private:
	int *current_index;
	
};

}

#endif
