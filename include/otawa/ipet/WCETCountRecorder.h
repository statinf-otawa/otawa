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

namespace otawa { namespace ipet {

// WCETCountRecorder class
class WCETCountRecorder: public BBProcessor {
	ilp::System *system;
protected:
	virtual void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
	virtual void setup(FrameWork *fw);
	virtual void cleanup(FrameWork *fw);
public:
	WCETCountRecorder(void);
};

// Features
extern Feature<WCETCountRecorder> WCET_COUNT_RECORDED_FEATURE;

} } // otawa::ipet

#endif	// OTAWA_IPET_WCET_COUNT_RECORDER_H
