/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/CFGProcessor.h -- CFGProcessor class interface.
 */
#ifndef OTAWA_PROC_CFGPROCESSOR_H
#define OTAWA_PROC_CFGPROCESSOR_H

#include <otawa/proc/Processor.h>
#include <otawa/cfg/CFG.h>

namespace otawa {
	
// Processor class
class CFGProcessor: public Processor {
public:
	virtual void processCFG(CFG *cfg) = 0;

	// Processor overload
	virtual void processFrameWork(FrameWork *fw);
};

} // otawa

#endif // OTAWA_PROC_CFGPROCESSOR_H
