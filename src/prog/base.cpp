/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	base.h -- base definition for Otawa framework.
 */

#include <stdarg.h>
#include <otawa/base.h>
using namespace elm;

namespace otawa {

/**
 * @class Exception
 * Base class of Otawa exceptions.
 */
 
/**
 * Build an empty exception.
 */
Exception::Exception(void) {
}

/**
 * Build a simple exception with the given message.
 * @param message	Message of the exception.
 */
Exception::Exception(const String& message): msg(message) {
}

/**
 * Build an exception by building a message using format and arguments
 * as the "printf" function.
 * @param format	Format of the message.
 * @param ...		Data use din the format string.
 */
Exception::Exception(const char *format, ...) {
	va_list args;
	va_start(args, format);
	build(format, args);
	va_end(args);
}


/**
 * Build an exception with "vprintf" style message building.
 * @param format	Format of the message.
 * @param args		Arguments used in the format.
 */
Exception::Exception(const char *format, va_list args) {
	build(format, args);
}

/**
 * Virtual destructor.
 */
Exception::~Exception(void) {
}


/**
 * Build the message using "printf" style format and arguments.
 */
void Exception::build(CString format, va_list args) {
	// !!TODO!!
}



/**
 * @fn const String& Exception::getMessage(void) const
 * Get the exception message.
 * @return	Exception message.
 */

}
