/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	base.h -- base definition for Otawa framework.
 */
#ifndef OTAWA_BASE_H
#define OTAWA_BASE_H

#include <stdarg.h>
#include <elm/io.h>
#include <elm/utility.h>
#include <elm/string.h>
#include <elm/util/VarArg.h>

namespace otawa {
using namespace elm;

// Base types
typedef unsigned char byte_t;
typedef unsigned long address_t ;
typedef unsigned long size_t;
typedef signed long offset_t;
typedef unsigned long mask_t;

// Exception class
class Exception: public elm::Exception {
	String msg;
protected:
	void build(elm::CString format, VarArg& args);
	inline void setMessage(elm::String message);
public:
	Exception(void);
	Exception(const String message);
	Exception(elm::CString format, elm::VarArg& args);
	Exception(elm::CString format, ...);
	virtual ~Exception(void);
	virtual String message(void) { return msg; };
};

// Inlines
inline void Exception::setMessage(elm::String message) {
	msg = message;
}

// Format
inline io::IntFormat address_format(address_t addr) {
	return elm::io::right(elm::io::width(8, elm::io::pad('0', elm::io::hex(addr))));
}

} // otawa

#endif	// OTAWA_BASE_H
