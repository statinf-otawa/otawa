/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/CachePenaltiesObjectFunctionBuilder.h -- CachePenaltiesObjectFunctionBuilder class interface.
 */
#ifndef OTAWA_IPET_CACHEPENALTIES_OBJECTFUNCTIONBUILDER_H
#define OTAWA_IPET_CACHEPENALTIES_OBJECTFUNCTIONBUILDER_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>
#include <otawa/cache/cat2/CachePenalty.h>

namespace otawa { namespace ipet {


// CachePenaltiesObjectFunctionBuilder class
class CachePenaltiesObjectFunctionBuilder: public BBProcessor {
public:
	CachePenaltiesObjectFunctionBuilder(void);

	// BBProcessor overload
	virtual void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
};

} } // otawa::ipet

#endif // OTAWA_IPET_CACHEPENALTIES_OBJECTFUNCTIONBUILDER_H
