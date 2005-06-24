/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/VarAssignment.h -- VarAssignment class interface.
 */
#ifndef OTAWA_IPET_VARASSIGNMENT_H
#define OTAWA_IPET_VARASSIGNMENT_H

#include <otawa/proc/BBProcessor.h>

namespace otawa {

class VarAssignment: public BBProcessor {
	bool _explicit;
	void process(BasicBlock *bb);
	String makeNodeVar(BasicBlock *bb);
	String makeEdgeVar(Edge *edge);
public:
	VarAssignment(void);

	// BBProcessor overload
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
	virtual void processCFG(FrameWork *fw, CFG *cfg);
	
	// Processor overload
	virtual void configure(PropList& props);
};

}	// otawa

#endif	// OTAWA_IPET_VARASSIGNMENT_H

