/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prop/NameSpace.h -- NameSpace class interface.
 */
#ifndef OTAWA_PROP_NAME_SPACE_H
#define OTAWA_PROP_NAME_SPACE_H

#include <elm/string.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/prop/AbstractIdentifier.h>

namespace otawa {

// NameSpace class
class NameSpace: public AbstractIdentifier {
	friend class AbstractIdentifier;
	NameSpace& _parent;
	elm::genstruct::HashTable<String, AbstractIdentifier *> names;
	elm::String _uri;
	void add(AbstractIdentifier *id);

public:
	NameSpace(elm::CString prefix, NameSpace& parent = ::NS);
	AbstractIdentifier *get(elm::String name);
	elm::String uri(void);
	
	// Identifier overload
	virtual NameSpace *toNameSpace(void);	
	virtual void print(elm::io::Output& out);
};

} // otawa

#endif	// OTAWA_PROP_NAME_SPACE_H
