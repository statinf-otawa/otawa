/*
 * ADTREE -- OTAWA plugin for Eclipse
 * Copyright (C) 2003-08  IRIT - UPS <casse@irit.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.package ojawa;
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
 * @ingroup prop
 */


/**
 * Build a new exception.
 * @param name	Name of identifier causing the exception.
 */
DuplicateIdentifierException::DuplicateIdentifierException(String& name)
: Exception(_ << "identifier \"" << name << "\" is already declared") {
};


/**
 * @class AbstractIdentifier
 * Represents a unique identifier used by the annotation system.
 * Identifier pointers may be compared for testing equality.
 * @ingroup prop
 */


/**
 * Find an abstract identifier by its name.
 * @param name	Name of the looked identifier.
 * @return	Found identifier or null.
 */
AbstractIdentifier *AbstractIdentifier::find(const string& name) {
	return NS.get(name);
}


/**
 * For internal use only.
 */
AbstractIdentifier *AbstractIdentifier::init_list = 0;
bool AbstractIdentifier::initialized = false;


/**
 * For internal use only.
 */
void AbstractIdentifier::init(void) {
	if(!initialized) {
		for(AbstractIdentifier *id = init_list; id; id = id->next)
			id->link();
		initialized = true;
	}
}


/**
 * For internal use only.
 */
void AbstractIdentifier::link(void) {
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
AbstractIdentifier::AbstractIdentifier(void)
:	nam(""),
	_parent(::NS)
{
		ASSERT(!((((unsigned int)this) > 0xbf000000) && (((unsigned int)this) < 0xc0000000)));
}


/**
 * Build a new identifier. Only one identifier may exists in the OTAWA with
 * a given name. If there is a name clash, the framework will immediatelly be
 * stopped.
 * @param name		Name of the identifier.
 * @param parent	Parent namespace.
 */
AbstractIdentifier::AbstractIdentifier(elm::String name, NameSpace& parent)
:	nam(name),
 	_parent(parent)
{
	ASSERT(!((((unsigned int)this) > 0xbf000000) && (((unsigned int)this) < 0xc0000000)));
	if(name) {
		if(initialized)
			link();
		else {
			next = init_list;
			init_list = this;
		}
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
NameSpace *AbstractIdentifier::toNameSpace(void) {
	return 0;
}


/**
 * @fn const elm::String& AbstractIdentifier::name(void) const;
 * Get the string name of the identifier.
 * @return	Identifier name.
 */


/**
 * <p>Print the value of the given property (accordint the property matches
 * the given identifier). It is an error to call this method with a property
 * matching a different identifier.</p>
 * <p>Use the print() method of a property instead.</p>
 */
void AbstractIdentifier::print(
	elm::io::Output& output,
	const Property& prop) const
{
	output << "<not printable>";
}

/**
 * @fn void AbstractIdentifier::print(elm::io::Output& output, const Property *prop) const;
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
const Type& AbstractIdentifier::type(void) const {
	return Type::no_type;
}


/**
 * Read the value of an identifier from the given variable arguments and
 * create the matching property in the given property list.
 * @param props	Property list to create property in.
 * @param args	Variable arguments to read identifier value from.
 * @warning		It is an error to call this method on a non-typed identifier.
 */
void AbstractIdentifier::scan(PropList& props, VarArg& args) const {
	assert(0);
}


/**
 * Print the identifier.
 * @param out	Output stream.
 */
void AbstractIdentifier::print(elm::io::Output& out) const {
	out << nam;
}


// Specialisation for types
template <>
const Type& Identifier<bool>::type(void) const { return Type::bool_type; }

template <>
const Type& Identifier<char>::type(void) const { return Type::char_type; }

template <>
const Type& Identifier<int>::type(void) const { return Type::int32_type; }

template <>
const Type& Identifier<long long>::type(void) const { return Type::int64_type; }

template <>
const Type& Identifier<char *>::type(void) const { return Type::cstring_type; }

} // otawa
