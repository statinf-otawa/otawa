/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicObjectFunctionBuilder.h -- BasicObjectFunctionBuilder class interface.
 */
#ifndef OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
#define OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa { namespace ipet {


// BasicObjectFunctionBuilder class
class BasicObjectFunctionBuilder: public CFGProcessor {
public:
	BasicObjectFunctionBuilder(const PropList& props = PropList::EMPTY);

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} } // otawa::ipet

#endif // OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
