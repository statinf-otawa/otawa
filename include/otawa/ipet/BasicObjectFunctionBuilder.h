/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/BasicObjectFunctionBuilder.h -- BasicObjectFunctionBuilder class interface.
 */
#ifndef OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
#define OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {


// BasicObjectFunctionBuilder class
class BasicObjectFunctionBuilder: public BBProcessor {
public:
	BasicObjectFunctionBuilder(void);

	// BBProcessor overload
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};

// Feature
extern Feature<BasicObjectFunctionBuilder> OBJECT_FUNCTION_FEATURE;

} } // otawa::ipet

#endif // OTAWA_IPET_BASICOBJECTFUNCTIONBUILDER_H
