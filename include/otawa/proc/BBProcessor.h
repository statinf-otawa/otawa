/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/proc/BBProcessor.h -- BBProcessor class interface.
 */
#ifndef OTAWA_PROC_BBPROCESSOR_H
#define OTAWA_PROC_BBPROCESSOR_H

#include <otawa/cfg/BasicBlock.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

class BBProcessor: public CFGProcessor {
protected:
	virtual void processCFG(FrameWork *fw, CFG *cfg);
	virtual void processBB(FrameWork *fw, CFG *cfd, BasicBlock *bb) = 0;
	
public:
	BBProcessor(void);
	BBProcessor(elm::String name, elm::Version version = elm::Version::ZERO);
};

} // otawa

#endif	// OTAWA_PROC_BBPROCESSOR_H
