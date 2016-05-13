#ifndef __OTAWA_OSLICE_INST_COLLECTOR_H__
#define __OTAWA_OSLICE_INST_COLLECTOR_H__

#include <otawa/cfg/features.h>
#include "oslice.h"
#include <otawa/prog/WorkSpace.h> // necessary to have Identifier working on fw

namespace otawa { namespace oslice {

class InstCollector: public otawa::Processor {
public:
	InstCollector(AbstractRegistration& _reg);

protected:
	// called by the children
	virtual void configure(const PropList &props);
	// implemented by each children
	void collectInterestedInstructions(const CFGCollection& coll, interested_instructions_t* interestedInstructions);
	virtual bool interested(Inst* i) = 0;

private:
	void processWorkSpace(WorkSpace *fw);
	int showDebugingMessage;
};

} } // otawa::oslice

#endif
