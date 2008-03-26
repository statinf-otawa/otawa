/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	AbstractIdentifier class interface
 */
#ifndef OTAWA_PROP_ABSTRACT_IDENTIFIER_H
#define OTAWA_PROP_ABSTRACT_IDENTIFIER_H

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
extern NameSpace NS;

} // otawa

// Root namespace
extern otawa::NameSpace NS;

namespace otawa {

// Identifier class
class AbstractIdentifier {
	friend class Manager;
	elm::String nam;
	NameSpace& _parent;
	AbstractIdentifier *next;

	static bool initialized;
	static AbstractIdentifier *init_list;
	static void init(void);
	void link(void);

public:	
	AbstractIdentifier(void);
	AbstractIdentifier(elm::String name, NameSpace& parent = ::NS);
	virtual ~AbstractIdentifier(void) { }

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
inline elm::io::Output& operator<<(elm::io::Output& out,
	const AbstractIdentifier& id)
{
	id.print(out);
	return out;
}

inline elm::io::Output& operator<<(elm::io::Output& out,
	const AbstractIdentifier *id)
{
	assert(id);
	return out << *id;
}


// Inlines
inline const elm::String AbstractIdentifier::name(void) const {
	return nam;
}

inline void AbstractIdentifier::print(elm::io::Output& output,
	const Property *prop) const
{
	assert(prop);
	return print(output, *prop);
}


} // otawa

#endif	// OTAWA_PROP_ABSTRACT_IDENTIFIER_H
