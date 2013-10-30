/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/TrivialBBTime.h -- TrivialBBTime class interface.
 */
#ifndef OTAWA_IPET_TRIVIALBBTIME_H
#define OTAWA_IPET_TRIVIALBBTIME_H

#include <elm/assert.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa { namespace ipet {

// TrivialBBTime class
class TrivialBBTime: public BBProcessor {
	unsigned dep;

protected:
	void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);

public:
	TrivialBBTime(void);
	virtual void configure(const PropList& props);
};

// Configuration Properties
extern Identifier<unsigned> PIPELINE_DEPTH;

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIALBBTIME_H
