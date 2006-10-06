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
#include <otawa/cfg/CFGCollector.h>

namespace otawa { 

// Externals
namespace ilp {
	class System;
} // ilp

namespace ipet {
	
// FlowFactLoader class
class FlowFactLoader: public Processor, private otawa::FlowFactLoader {
	CFGCollection *cfgs;	
	otawa::ilp::System *system;
	bool dom_done;
protected:
	// FlowFactLoader overload
	virtual void onError(const char *fmt, ...);
	virtual void onLoop(address_t addr, int count);	
public:
	FlowFactLoader(const PropList& props = PropList::EMPTY);

	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
};


} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
