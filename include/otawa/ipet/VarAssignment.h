/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	VarAssignment class interface.
 */
#ifndef OTAWA_IPET_VARASSIGNMENT_H
#define OTAWA_IPET_VARASSIGNMENT_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// VarAsignment class
class VarAssignment: public BBProcessor {
	bool _explicit, _recursive;
	String makeNodeVar(BasicBlock *bb, CFG *cfg);
	String makeEdgeVar(Edge *edge, CFG *cfg);

protected:
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);

public:
	VarAssignment(void);
	virtual void configure(const PropList& props);
};

// Features
extern Feature<VarAssignment> ASSIGNED_VARS_FEATURE;

} } // otawa::ipet

#endif	// OTAWA_IPET_VARASSIGNMENT_H

