/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prop/Identifier.h -- interface to Identifier class.
 */
#ifndef OTAWA_PROP_IDENTIFIER_H
#define OTAWA_PROP_IDENTIFIER_H

#include <assert.h>
#include <elm/string.h>
#include <otawa/base.h>

namespace otawa {

// Identifier class
class Identifier {
	elm::String nam;
public:
	Identifier(elm::CString name);
	inline const elm::String& name(void) const { return nam; };
	static Identifier *getID(elm::CString name);
	static const Identifier *invalid;
};


// DuplicateIdentifierException class
class DuplicateIdentifierException: public Exception {
public:
	DuplicateIdentifierException(String& name);
};


// Compatibility
typedef Identifier *id_t;
#define INVALID_ID Identifier::invalid

} // otawa

#endif	// OTAWA_PROP_IDENTIFIER_H
