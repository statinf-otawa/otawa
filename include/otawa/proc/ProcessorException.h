/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/ProcessorException.h -- ProcessorException class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_EXCEPTION_H
#define OTAWA_PROC_PROCESSOR_EXCEPTION_H

#include <otawa/base.h>

namespace otawa {

class Processor;

// ProcessorException class
class ProcessorException: public otawa::Exception {
public:
	ProcessorException(const Processor& proc, const elm::String& message);
};

} // otawa

#endif //OTAWA_PROC_PROCESSOR_EXCEPTION_H
