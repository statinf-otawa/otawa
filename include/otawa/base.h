/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	base.h -- base definition for Otawa framework.
 */
#ifndef OTAWA_BASE_H
#define OTAWA_BASE_H

#include <stdarg.h>
#include <elm/utility.h>
#include <elm/string.h>

namespace otawa {
using namespace elm;

// Exception class
class Exception: public elm::Exception {
	String msg;
protected:
	void build(CString format, va_list args);
public:
	Exception(void);
	Exception(const String& message);
	Exception(const char *format, va_list args);
	Exception(const char *format, ...);
	virtual ~Exception(void);
	virtual String message(void) { return msg; };
};

// Base types
typedef unsigned char byte_t;
typedef byte_t *address_t ;
typedef unsigned long size_t;
typedef signed long offset_t;


}

#endif	// OTAWA_BASE_H
