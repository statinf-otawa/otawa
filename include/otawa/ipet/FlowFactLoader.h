/*
 * $Id$
 * Copyright (c) 2005-06, IRIT-UPS <casse@irit.fr>
 *
 * otawa/ipet/IPETFlowFactLoader.h -- IPETFlowFactLoader class interface.
 */
#ifndef OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
#define OTAWA_IPET_IPET_FLOW_FACT_LOADER_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { 

using namespace elm;

// Externals
namespace ilp {
	class System;
} // ilp

namespace ipet {
	
// FlowFactLoader class
class FlowFactLoader: public BBProcessor {
public:
	FlowFactLoader(void);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);

private:
	bool lines_available;
	bool transfer(Inst *source, BasicBlock *bb);
};

// Features
extern Feature<FlowFactLoader> FLOW_FACTS_FEATURE;

} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
