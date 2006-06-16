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
#include <elm/io.h>
#include <elm/util/VarArg.h>
#include <otawa/base.h>
#include <otawa/type.h>

namespace otawa {

// External classes
class Property;
class PropList;

// Identifier class
class Identifier {
	elm::String nam;
public:
	static const Identifier *invalid;
	static const Identifier *getID(elm::CString name);
	Identifier(elm::CString name);
	
	inline const elm::String& name(void) const;
	virtual void print(elm::io::Output& output, const Property& prop);
	inline void print(elm::io::Output& output, const Property *prop);
	virtual const Type& type(void) const;
	virtual void scan(PropList& props, VarArg& args) const;
};


// DuplicateIdentifierException class
class DuplicateIdentifierException: public Exception {
public:
	DuplicateIdentifierException(String& name);
};


// Compatibility
#define INVALID_ID Identifier::invalid


// Inlines
inline const elm::String& Identifier::name(void) const {
	return nam;
}

inline void Identifier::print(elm::io::Output& output, const Property *prop) {
	assert(prop);
	return print(output, *prop);
}

} // otawa

#endif	// OTAWA_PROP_IDENTIFIER_H
