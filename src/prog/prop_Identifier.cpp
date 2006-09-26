/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	prog/prop_PropList.cpp -- implementation of Identifier class.
 */

#include <elm/io.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/properties.h>
#include <otawa/prop/NameSpace.h>

using namespace elm;

namespace otawa {


/*
 * NOTE : init_list, initialized, init and link are workaround the C++
 * global constructor order FIASCO. Modify it only if you know what you do.
 * 
 * At construction time, the has table are not filled because the parent
 * namespace is possibly not initialized. The identifier records them in 
 * single link list (statically initialized to NULL... no construction).
 * Then, the init() function is called and the identifiers recorded in the
 * list ends their construction by recording themselves to their parent
 * namespaces.
 */


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
 * @class Identifier otawa/properties.h
 * Represents a unique identifier used by the annotation system.
 * Identifier pointers may be compared for testing equality.
 */


/**
 * For internal use only.
 */
Identifier *Identifier::init_list = 0;
bool Identifier::initialized = false;


/**
 * For internal use only.
 */
void Identifier::init(void) {
	if(!initialized) {
		for(Identifier *id = init_list; id; id = id->next)
			id->link();
		initialized = true;
	}
}


/**
 * For internal use only.
 */
void Identifier::link(void) {
	if(_parent.get(nam)) {
		cerr << "FATAL ERROR: identifier \"" << nam << "\" defined multiple times.";
		String _(nam);
		throw DuplicateIdentifierException(_);
	}
	_parent.add(this);
}


/**
 * Build an aninymouns identifier.
 */
Identifier::Identifier(void)
:	nam(""),
	_parent(ROOT_NS)
{
}


/**
 * Build a new identifier. Only one identifier may exists in the OTAWA with
 * a given name. If there is a name clash, the framework will immediatelly be
 * stopped.
 * @param name		Name of the identifier.
 * @param parent	Parent namespace.
 */
Identifier::Identifier(elm::String name, NameSpace& parent)
:	nam(name),
 	_parent(parent)
{
	if(initialized)
		link();
	else {
		next = init_list;
		init_list = this;
	}
}


/**
 * @fn NameSpace const & Identifier::parent(void) const;
 * Get the parent namespace.
 * @return Parent namespace.
 */


/**
 * If this identifier is a namespace, return it.
 * @return	Matching namespace or null.
 */
NameSpace *Identifier::toNameSpace(void) {
	return 0;
}


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
void Identifier::print(elm::io::Output& output, const Property& prop) const {
	output << "<not printable>";
}

/**
 * @fn void Identifier::print(elm::io::Output& output, const Property *prop) const;
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
void Identifier::scan(PropList& props, VarArg& args) const {
	assert(0);
}


/**
 * Print the identifier.
 * @param out	Output stream.
 */
void Identifier::print(elm::io::Output& out) {
	out << nam;
}


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
