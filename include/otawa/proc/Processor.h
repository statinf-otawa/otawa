/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/Processor.h -- Processor class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_H
#define OTAWA_PROC_PROCESSOR_H

#include <elm/io.h>
#include <otawa/prog/FrameWork.h>

namespace otawa {

// Processor class
class Processor {
protected:
	elm::io::Output out;
public:
	static Identifier ID_Output;

	virtual void configure(PropList& props);
	virtual void processFrameWork(FrameWork *fw) = 0;
};

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
