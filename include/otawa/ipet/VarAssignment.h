/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/VarAssignment.h -- VarAssignment class interface.
 */
#ifndef OTAWA_IPET_VARASSIGNMENT_H
#define OTAWA_IPET_VARASSIGNMENT_H

#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace ipet {

// VarAsignment class
class VarAssignment: public BBProcessor {
	bool _explicit, _recursive;
	String makeNodeVar(BasicBlock *bb);
	String makeEdgeVar(Edge *edge);
	void init(const PropList& props);

protected:
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);

public:
	VarAssignment(const PropList& props = PropList::EMPTY);
	virtual void configure(PropList& props);
};

} } // otawa::ipet

#endif	// OTAWA_IPET_VARASSIGNMENT_H

