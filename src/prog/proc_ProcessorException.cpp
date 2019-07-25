/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_ProcessorException.cpp -- ProcessorException class implementation.
 */

#include <otawa/proc/Processor.h>
#include <otawa/proc/ProcessorException.h>
#include <elm/debug.h>

using namespace elm;

namespace otawa {

/**
 * @class ProcessorException
 * This class is used for returning exceptions from the processors.
 * @ingroup proc
 */

/**
 * Build a processor exception with a simple message.
 * @param proc		Processor throwing the exception.
 * @param message	Exception message.
 */
ProcessorException::ProcessorException(
	const Processor& proc,
	const elm::String& message
): Exception(_
	<< proc.name() << " (" << proc.version() << "):"
	<< message
) {
}

} // otawa
