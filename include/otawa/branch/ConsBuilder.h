#ifndef BRANCH_CONSBUILDER_H_
#define BRANCH_CONSBUILDER_H_

#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/BasicBlock.h>

#include "BranchBuilder.h"

namespace otawa {
	
using namespace elm;

class ConsBuilder: public BBProcessor {
	public:
	ConsBuilder(void);
	virtual void processBB(otawa::WorkSpace*, CFG *cfg, BasicBlock *bb);
	virtual void configure(const PropList &props);
	
	private:
	bool _explicit;

};

}

#endif
