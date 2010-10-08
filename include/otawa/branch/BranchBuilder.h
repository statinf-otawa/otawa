#ifndef BRANCH_BRANCHBUILDER_H_
#define BRANCH_BRANCHBUILDER_H_

#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/BasicBlock.h>

#include "BranchProblem.h"

namespace otawa {
	
using namespace elm;

    typedef enum branch_category_t {
	BRANCH_ALWAYS_D,
	BRANCH_ALWAYS_H,
	BRANCH_FIRST_UNKNOWN,
	BRANCH_NOT_CLASSIFIED
    } branch_category_t;


extern Identifier<branch_category_t> BRANCH_CATEGORY;
extern Identifier<BasicBlock*> BRANCH_HEADER;

class BranchBuilder : public otawa::Processor {

	public:
	BranchBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);
	
	private:
	void categorize(BasicBlock *bb, BranchProblem::Domain *dom, BasicBlock* &header, branch_category_t &cat);	
};

}

#endif
