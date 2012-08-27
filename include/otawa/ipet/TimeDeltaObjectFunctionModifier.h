/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 */
#ifndef OTAWA_IPET_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H
#define OTAWA_IPET_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {


// TimeDeltaObjectFunctionModifier class
class TimeDeltaObjectFunctionModifier: public BBProcessor {
public:
	static p::declare reg;
	TimeDeltaObjectFunctionModifier(p::declare& r = reg);

	// BBProcessor overload
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
};

// Features
extern Feature<TimeDeltaObjectFunctionModifier> EDGE_TIME_FEATURE;

} } // otawa::ipet

#endif // OTAWA_IPET_TIME_DELTA_OBJECT_FUNCTION_MODIFIER_H
