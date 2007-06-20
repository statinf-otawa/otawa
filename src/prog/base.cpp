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
Exception::Exception(void): MessageException("") {
}

/**
 * Build a simple exception with the given message.
 * @param message	Message of the exception.
 */
Exception::Exception(const string& message): MessageException(message) {
}


/**
 * @fn const String& Exception::getMessage(void) const
 * Get the exception message.
 * @return	Exception message.
 */


/**
 * @class Address
 */
Address Address::null;

}
