/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 *
 * otawa/ipet/IPETFlowFactLoader.h -- IPETFlowFactLoader class interface.
 */
#ifndef OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
#define OTAWA_IPET_IPET_FLOW_FACT_LOADER_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/util/FlowFactLoader.h>

namespace otawa { 

// Externals
namespace ilp {
	class System;
} // ilp

namespace ipet {
	
// FlowFactLoader class
class FlowFactLoader: public CFGProcessor, private otawa::FlowFactLoader {
	CFG *cfg;	
	otawa::ilp::System *system;
	bool dom_done;
protected:
	// FlowFactLoader overload
	virtual void onError(const char *fmt, ...);
	virtual void onLoop(address_t addr, int count);	
public:

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};


} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
