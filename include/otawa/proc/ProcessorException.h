/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/ProcessorException.h -- ProcessorException class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_EXCEPTION_H
#define OTAWA_PROC_PROCESSOR_EXCEPTION_H

#include <otawa/proc/Processor.h>

namespace otawa {

// ProcessorException class
class ProcessorException: public otawa::Exception {
	String build(const Processor& proc, elm::String message);
	String build(const Processor& proc, elm::CString format, elm::VarArg& args);
public:
	ProcessorException(const Processor& proc, elm::String& message);
	ProcessorException(const Processor& proc, elm::CString format, ...);
	ProcessorException(const Processor& proc, elm::CString format,
		elm::VarArg& args);
};

} // otawa

#endif //OTAWA_PROC_PROCESSOR_EXCEPTION_H
