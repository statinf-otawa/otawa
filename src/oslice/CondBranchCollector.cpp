#include <otawa/oslice/CondBranchCollector.h>

namespace otawa { namespace oslice {

p::feature COND_BRANCH_COLLECTOR_FEATURE("otawa::oslice::COND_BRANCH_COLLECTOR_FEATURE", new Maker<CondBranchCollector>());

Registration<CondBranchCollector> CondBranchCollector::reg(
	"otawa::oslice::CondBranchCollector", Version(1, 0, 0),
	p::require, &COLLECTED_CFG_FEATURE,
	p::provide, &COND_BRANCH_COLLECTOR_FEATURE,
	p::provide, &INSTRUCTION_COLLECTOR_FEATURE,
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




class UnknownTargetCollector: public InstCollector {
public:
	static Registration<UnknownTargetCollector> reg;
	UnknownTargetCollector(AbstractRegistration& _reg = reg);

protected:
	virtual void configure(const PropList &props);
	virtual bool interested(Inst* i);
	virtual void cleanup(WorkSpace *ws);
};

/**
 */
UnknownTargetCollector::UnknownTargetCollector(AbstractRegistration& _reg)
: 	InstCollector(_reg) { }

/**
 */
void UnknownTargetCollector::configure(const PropList &props) {
	InstCollector::configure(props);
}

bool UnknownTargetCollector::interested(Inst* i) {
//	elm::cout << "inst " << i << " with kind " << hex(i->kind()) << io::endl;
	if(i->isControl() && !i->target() && !i->isReturn())
		return true;
	else
		return false;
}

void UnknownTargetCollector::cleanup(WorkSpace *ws) {
	addCleaner(UNKNOWN_TARGET_COLLECTOR_FEATURE, new InstCollectorCleaner(ws, interestedInstructionsLocal));
}


p::feature UNKNOWN_TARGET_COLLECTOR_FEATURE("otawa::oslice::UNKNOWN_TARGET_COLLECTOR_FEATURE", new Maker<UnknownTargetCollector>());

Registration<UnknownTargetCollector> UnknownTargetCollector::reg(
	"otawa::oslice::UnknownTargetCollector", Version(1, 0, 0),
	p::require, &COLLECTED_CFG_FEATURE,
	p::provide, &UNKNOWN_TARGET_COLLECTOR_FEATURE,
	p::provide, &INSTRUCTION_COLLECTOR_FEATURE,
	p::end
);

} }
