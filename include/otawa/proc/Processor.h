/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/Processor.h -- Processor class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_H
#define OTAWA_PROC_PROCESSOR_H

#include <otawa/prog/FrameWork.h>

namespace otawa {
	
// Processor class
class Processor {
public:
	virtual void processFrameWork(FrameWork *fw) = 0;
};

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
