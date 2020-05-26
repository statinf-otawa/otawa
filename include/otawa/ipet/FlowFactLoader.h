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
	void setup(WorkSpace *ws) override;
	void cleanup (WorkSpace *fw) override;
	void processBB(WorkSpace *ws, CFG *cfg, Block *bb, const ContextualPath& path) override;

private:
	bool lines_available;
	int total_loop, found_loop, line_loop;
	int max, total, min;
	bool isIntoConstraint;//MDM 
	/*genstruct::*/Vector<  Block *> seenFunction;//MDM endListContraint
	/*genstruct::*/Vector <Pair < ContextualPath, ContextualPath> >  ListOfPathOfBenginingOfConstraint;//MDM  

	 
	void transferConflict(Inst *source, /*BasicBlock*/Block *bb, const ContextualPath& path, bool intoLoop);//MDM
	
	DomInfo *dom;

	bool scan(Block *v, Block *t, const ContextualPath& path);
	bool transfer(Inst *source, Block *bb, const ContextualPath& path);
	bool lookLineAt(Inst *source, Block *bb, const ContextualPath& path);
};

} } // otawa::ipet

#endif // OTAWA_IPET_IPET_FLOW_FACT_LOADER_H
