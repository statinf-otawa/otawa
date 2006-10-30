/*
 *	$Id$
 *	Copyright (c) 2003-06, IRIT UPS.
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
class NameSpace;

// External Data
extern NameSpace ROOT_NS;
extern NameSpace OTAWA_NS;

// Identifier class
class Identifier {
	friend class Manager;
	elm::String nam;
	NameSpace& _parent;
	Identifier *next;

	static bool initialized;
	static Identifier *init_list;
	static void init(void);
	void link(void);

public:	
	Identifier(void);
	Identifier(elm::String name, NameSpace& parent = ROOT_NS);

	inline NameSpace const & parent(void) const { return _parent; };
	virtual NameSpace *toNameSpace(void);
	inline const elm::String name(void) const;

	virtual void print(elm::io::Output& out) const;
	virtual void print(elm::io::Output& output, const Property& prop) const;
	inline void print(elm::io::Output& output, const Property *prop) const;
	virtual const Type& type(void) const;
	virtual void scan(PropList& props, VarArg& args) const;
};


// DuplicateIdentifierException class
class DuplicateIdentifierException: public Exception {
public:
	DuplicateIdentifierException(String& name);
};


// Output
inline elm::io::Output& operator<<(elm::io::Output& out, const Identifier& id) {
	id.print(out);
	return out;
}

inline elm::io::Output& operator<<(elm::io::Output& out, const Identifier *id) {
	assert(id);
	return out << *id;
}


// Inlines
inline const elm::String Identifier::name(void) const {
	return nam;
}

inline void Identifier::print(elm::io::Output& output, const Property *prop) const {
	assert(prop);
	return print(output, *prop);
}


} // otawa

#endif	// OTAWA_PROP_IDENTIFIER_H
