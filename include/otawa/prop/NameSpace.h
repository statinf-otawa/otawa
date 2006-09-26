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
#include <otawa/prop/Identifier.h>

namespace otawa {

// NameSpace class
class NameSpace: public Identifier {
	friend class Identifier;
	NameSpace& _parent;
	elm::genstruct::HashTable<String, Identifier *> names;
	elm::String _uri;
	void add(Identifier *id);

public:
	NameSpace(elm::CString prefix, NameSpace& parent = ROOT_NS);
	Identifier *get(elm::String name);
	elm::String uri(void);
	
	// Identifier overload
	virtual NameSpace *toNameSpace(void);	
	virtual void print(elm::io::Output& out);
};

} // otawa

#endif	// OTAWA_PROP_NAME_SPACE_H
