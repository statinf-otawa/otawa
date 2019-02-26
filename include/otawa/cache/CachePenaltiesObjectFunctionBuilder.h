/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/CachePenaltiesObjectFunctionBuilder.h -- CachePenaltiesObjectFunctionBuilder class interface.
 */
#ifndef OTAWA_IPET_CACHEPENALTIES_OBJECTFUNCTIONBUILDER_H
#define OTAWA_IPET_CACHEPENALTIES_OBJECTFUNCTIONBUILDER_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>
#include <otawa/cache/cat2/CachePenalty.h>

namespace otawa { namespace ipet {


// CachePenaltiesObjectFunctionBuilder class
class CachePenaltiesObjectFunctionBuilder: public BBProcessor {
public:
	static p::declare reg;
	CachePenaltiesObjectFunctionBuilder(void);

	virtual void configure(const PropList& props = PropList::EMPTY);

protected:
	bool _explicit;
	DomInfo *dom;

	void setup(WorkSpace *ws) override;
	void processBB(WorkSpace *fw, CFG *cfg, Block *bb) override;
};

} } // otawa::ipet

#endif // OTAWA_IPET_CACHEPENALTIES_OBJECTFUNCTIONBUILDER_H
