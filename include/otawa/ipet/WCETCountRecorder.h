/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	WCETCountRecorder class interface.
 */
#ifndef OTAWA_IPET_WCET_COUNT_RECORDER_H
#define OTAWA_IPET_WCET_COUNT_RECORDER_H

#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/Feature.h>
#include <otawa/ilp.h>

namespace otawa { namespace ipet {

// WCETCountRecorder class
class WCETCountRecorder: public BBProcessor {
	ilp::System *system;
protected:
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
public:
	WCETCountRecorder(void);
};

} } // otawa::ipet

#endif	// OTAWA_IPET_WCET_COUNT_RECORDER_H
