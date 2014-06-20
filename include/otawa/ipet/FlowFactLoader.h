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
#include <otawa/prop/ContextualProperty.h>
#include <otawa/proc/ContextualProcessor.h>

namespace otawa { 

using namespace elm;

// Externals
namespace ilp {
	class System;
} // ilp

namespace ipet {
	
// FlowFactLoader class
class FlowFactLoader: public ContextualProcessor {
public:
	static p::declare reg;
	FlowFactLoader(p::declare& r = reg);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void cleanup (WorkSpace *fw);
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
	virtual void enteringCall (WorkSpace *ws, CFG *cfg, BasicBlock *caller, BasicBlock *callee);
	virtual void leavingCall (WorkSpace *ws, CFG *cfg, BasicBlock *to);

private:
	bool lines_available;
	int total_loop, found_loop, line_loop;
	int max, total, min;
	ContextualPath path;

	bool transfer(Inst *source, BasicBlock *bb);
	bool lookLineAt(Inst *source, BasicBlock *bb);
};

} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
