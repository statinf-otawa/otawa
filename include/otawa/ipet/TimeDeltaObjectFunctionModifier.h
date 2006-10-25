/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 */
#ifndef OTAWA_IPET_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H
#define OTAWA_IPET_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace ipet {


// TimeDeltaObjectFunctionModifier class
class TimeDeltaObjectFunctionModifier: public BBProcessor {
public:
	TimeDeltaObjectFunctionModifier(const PropList& props = PropList::EMPTY);

	// BBProcessor overload
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};

} } // otawa::ipet

#endif // OTAWA_IPET_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H
