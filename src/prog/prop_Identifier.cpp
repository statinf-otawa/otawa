/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	prog/prop_PropList.cpp -- implementation of Identifier class.
 */

#include <elm/io.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/properties.h>
using namespace elm;

namespace otawa {

// Property names store
static genstruct::HashTable<String, Identifier *> *ids = 0;


/**
 * @class DuplicateIdentifierException
 * This exception is thrown when two identifier with the same name are
 * declared.
 */


/**
 * Build a new exception.
 * @param name	Name of identifier causing the exception.
 */
DuplicateIdentifierException::DuplicateIdentifierException(String& name)
: Exception("Identifier \"%s\" is already declared.", &name.toCString()) {
};


/**
 * @class Identifier
 * Represents a unique identifier used by the annotation system.
 * Identifier pointer may be compared for testing equality.
 */
Identifier::Identifier(CString name): nam(name) {
	
	// Need to allocate the ID table ?
	if(!ids)
		ids = new genstruct::HashTable<String, Identifier *>();
	
	// Already defined
	Option<Identifier *> result = ids->get(nam);
	if(result)
		throw DuplicateIdentifierException(nam);
	
	// Add it
	ids->put(name, this);
}


/**
 * Get the identifier matching the given name.
 * @param name	Identifier name.
 * @return		Matching identifier.
 */
Identifier *Identifier::getID(CString name) {
	String nam(name);
	
	// Test for existence
	if(ids) {
		Option<Identifier *> res = ids->get(nam);
		if(res)
			return *res;
		else
		ids = new genstruct::HashTable<String, Identifier *>();		
	}
	
	// Else create it
	return new Identifier(name);
}


/**
 * Value for the undefined identifier.
 */
const Identifier *Identifier::invalid = 0;


/**
 * @fn const elm::String& Identifier::name(void) const;
 * Get the string name of the identifier.
 * @return	Identifier name.
 */

} // otawa
