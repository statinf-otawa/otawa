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
 * Build a processor exception with a simple message.
 * @param proc		Processor throwing the exception.
 * @param message	Exception message.
 */
ProcessorException::ProcessorException(
	const Processor& proc,
	const elm::String& message
) {
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
	setMessage(buffer.toString());
}

} // otawa
