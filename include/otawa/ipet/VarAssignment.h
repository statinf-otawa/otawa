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
public:

	// BBProcessor overload
	virtual void processBB(BasicBlock *bb);
};

}	// otawa

#endif	// OTAWA_IPET_VARASSIGNMENT_H

