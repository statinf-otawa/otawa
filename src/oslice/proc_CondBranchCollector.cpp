#include "otawa/oslice/oslice_CondBranchCollector.h"

namespace otawa { namespace oslice {

p::feature COND_BRANCH_COLLECTOR_FEATURE("otawa::oslice::COND_BRANCH_COLLECTOR_FEATURE", new Maker<CondBranchCollector>());

Registration<CondBranchCollector> CondBranchCollector::reg(
	"otawa::oslice::CondBranchCollector", Version(1, 0, 0),
	p::use, &COLLECTED_CFG_FEATURE,
	p::provide, &COND_BRANCH_COLLECTOR_FEATURE,
	p::end
);

/**
 */
CondBranchCollector::CondBranchCollector(AbstractRegistration& _reg)
: 	InstCollector(_reg) { }

/**
 */
void CondBranchCollector::configure(const PropList &props) {
	InstCollector::configure(props);
}

bool CondBranchCollector::interested(Inst* i) {
	if(i->isConditional() && i->isControl())
		return true;
	else
		return false;
}

} }
