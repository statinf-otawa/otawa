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
	interested_instructions_t *interestedInstructionsLocal;


private:
	void processWorkSpace(WorkSpace *fw);
	int showDebugingMessage;
};

/*
 * The Cleaner class for InstCollector
 */
class InstCollectorCleaner: public Cleaner {
public:
	InstCollectorCleaner(WorkSpace *_ws, interested_instructions_t* _iiti): ws(_ws), interestedInstructionsLocal(_iiti) { }

protected:
	virtual void clean(void) {
//		interested_instructions_t* targetToRemove = INTERESTED_INSTRUCTIONS(ws);
//		for(interested_instructions_t::Iterator iiti(*interestedInstructionsLocal); iiti; iiti++) {
//			targetToRemove->remove(iiti);
//		}
		INTERESTED_INSTRUCTIONS(ws) = 0;
	}
private:
	WorkSpace* ws;
	interested_instructions_t* interestedInstructionsLocal;
};

} } // otawa::oslice

#endif
