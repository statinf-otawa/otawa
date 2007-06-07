/*
 * $Id$
 * Copyright (c) 2005-06, IRIT-UPS <casse@irit.fr>
 *
 * otawa/ipet/IPETFlowFactConstraintBuilder.h -- IPETFlowFactConstraintBuilder class interface.
 */
#ifndef OTAWA_IPET_IPET_FLOW_FACT_CONSTRAINT_BUILDER_H
#define OTAWA_IPET_IPET_FLOW_FACT_CONSTRAINT_BUILDER_H

#include <elm/system/Path.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/proc/Feature.h>

namespace otawa { 

using namespace elm;

// Externals
namespace ilp {
	class System;
} // ilp

namespace ipet {
	
// FlowFactConstraintBuilder class
class FlowFactConstraintBuilder: public CFGProcessor {


	// CFGProcessor overload
	virtual void processCFG(WorkSpace *fw, CFG *cfg);

public:
	FlowFactConstraintBuilder(void);
};

// Features
extern Feature<FlowFactConstraintBuilder> FLOW_FACTS_CONSTRAINTS_FEATURE;

} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_CONSTRAINT_BUILDER_H
