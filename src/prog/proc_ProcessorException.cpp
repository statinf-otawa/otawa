/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_ProcessorException.cpp -- ProcessorException class implementation.
 */

#include <otawa/proc/Processor.h>
#include <elm/debug.h>

using namespace elm;

namespace otawa {

/**
 * @class ProcessorException
 * This class is used for returning exceptions from the processors.
 */


/**
 */
String ProcessorException::build(const Processor& proc, elm::String message) {
	StringBuffer buffer;
	if(!proc.name())
		buffer << "no name";
	else {
		buffer << proc.name();
		if(proc.version())
			buffer << '(' << proc.version() << ')';
	}
	buffer << ": ";
	buffer << message;
	return buffer.toString();
}


/**
 */
String ProcessorException::build(const Processor& proc, elm::CString format,
elm::VarArg& args) {
	StringBuffer buffer;
	buffer.format(format, args);
	return build(proc, buffer.toString());	
}


/**
 * Build a processor exception with a simple message.
 * @param proc		Processor throwing the exception.
 * @param message	Exception message.
 */
ProcessorException::ProcessorException(const Processor& proc,
elm::String& message) {
	setMessage(build(proc, message));
}


/**
 * Build a processor exception with a-la C formatted message.
 * @param proc		Processor throwing the exception.
 * @param format	Format string.
 * @param ...		Variable arguments.
 */
ProcessorException::ProcessorException(const Processor& proc,
elm::CString format, ...) {
	VARARG_BEGIN(args, format)
		setMessage(build(proc, format, args));
	VARARG_END
}


/**
 * Build a processor exception with a-la C formatted message.
 * @param proc		Processor throwing the exception.
 * @param format	Format string.
 * @param args		Variable arguments.
 */
ProcessorException::ProcessorException(const Processor& proc,
elm::CString format, elm::VarArg& args) {
	setMessage(build(proc, format, args));
}

} // otawa
