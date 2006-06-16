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
	if(result) {
		cerr << "FATAL ERROR: identifier \"" << nam << "\" defined multiple times.";
		throw DuplicateIdentifierException(nam);
	}
	
	// Add it
	ids->put(name, this);
}


/**
 * Get the identifier matching the given name.
 * @param name	Identifier name.
 * @return		Matching identifier.
 */
const Identifier *Identifier::getID(CString name) {
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


/**
 * <p>Print the value of the given property (accordint the property matches
 * the given identifier). It is an error to call this method with a property
 * matching a different identifier.</p>
 * <p>Use the print() method of a property instead.</p>
 */
void Identifier::print(elm::io::Output& output, const Property& prop) {
	output << "<not printable>";
}

/**
 * @fn void Identifier::print(elm::io::Output& output, const Property *prop);
 * <p>Print the value of the given property (accordint the property matches
 * the given identifier). It is an error to call this method with a property
 * matching a different identifier.</p>
 * <p>Use the print() method of a property instead.</p>
 */


/**
 * Get the identifier of data linked with this property. It may return @ref
 * Type::no_type ever meaning that the identifier does not support type system
 * or that it is just a flag without associated data.
 * @return	Type of the associated data.
 */
const Type& Identifier::type(void) const {
	return Type::no_type;
}


/**
 * Read the value of an identifier from the given variable arguments and
 * create the matching property in the given property list.
 * @param props	Property list to create property in.
 * @param args	Variable arguments to read identifier value from.
 * @warning		It is an error to call this method on a non-typed identifier.
 */
/*void Identifier::scan(PropList& props, VarArg& args) const {
	assert(0);
}*/


// Specialisation for types
template <>
const Type& GenericIdentifier<bool>::type(void) const { return Type::bool_type; }

template <>
const Type& GenericIdentifier<char>::type(void) const { return Type::char_type; }

template <>
const Type& GenericIdentifier<int>::type(void) const { return Type::int32_type; }

template <>
const Type& GenericIdentifier<long long>::type(void) const { return Type::int64_type; }

template <>
const Type& GenericIdentifier<char *>::type(void) const { return Type::cstring_type; }

} // otawa
