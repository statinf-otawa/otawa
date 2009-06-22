/*
 * OTAWA -- WCET computation framework
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
#include <elm/util/Initializer.h>

using namespace elm;

#define TRACE(txt) //cerr << txt << io::endl;

namespace otawa {

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
: otawa::Exception(_ << "identifier \"" << name << "\" is already declared") {
};


// Storage of known identifiers
static genstruct::HashTable<String, AbstractIdentifier *> ids;
static Initializer<AbstractIdentifier> ids_init;


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
	return ids.get(name, 0);
}


/**
 * Build an anonymous identifier.
 */
AbstractIdentifier::AbstractIdentifier(void)
:	nam("") {
		//ASSERT(!((((unsigned int)this) > 0xbf000000) && (((unsigned int)this) < 0xc0000000)));
}


/**
 * Build a new identifier. Only one identifier may exists in the OTAWA with
 * a given name. If there is a name clash, the framework will immediatelly be
 * stopped.
 * @param name		Name of the identifier.
 * @param parent	Parent namespace.
 */
AbstractIdentifier::AbstractIdentifier(elm::String name)
:	nam(name)
{
	//ASSERT(!((((unsigned int)this) > 0xbf000000) && (((unsigned int)this) < 0xc0000000)));
	TRACE("construct(" << (void *)this << ", " << nam << ")");
	if(name) {
		TRACE("record(" << (void *)this << ", " << nam << ")");
		ids_init.record(this);
	}
}


/**
 * @fn const elm::String& AbstractIdentifier::name(void) const;
 * Get the string name of the identifier.
 * @return	Identifier name.
 */


/**
 * Print the identifier.
 * @param out	Output stream.
 */
void AbstractIdentifier::print(elm::io::Output& out) const {
	out << nam;
}


/**
 * Print the value of the given property (accordint the property matches
 * the given identifier). It is an error to call this method with a property
 * matching a different identifier.
 *
 * Use the print() method of a property instead.
 */
void AbstractIdentifier::print(io::Output& output, const Property& prop) const {
	output << "<not printable>";
}


/**
 * @fn void AbstractIdentifier::print(elm::io::Output& output, const Property *prop) const;
 * Print the value of the given property (accordint the property matches
 * the given identifier). It is an error to call this method with a property
 * matching a different identifier.
 *
 * Use the print() method of a property instead.
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
	ASSERTP(0, "scan() not available for identifier " << name());
}


/**
 * Get value of an identifier from a string and store it in the given
 * property list.
 * @param props	Property list to store result into.
 * @param str	String to get value from.
 */
void AbstractIdentifier::fromString(PropList& props, const string& str) const {
	ASSERTP(0, "fromString() not available for identifier " << name());
}


/**
 * For internal use only.
 */
void AbstractIdentifier::initialize(void) {
	TRACE("initialize(" << (void *)this << ", " << nam << ")");
	if(ids.get(nam)) {
		cerr << "FATAL ERROR: identifier \"" << nam << "\" defined multiple times.";
		String _(nam);
		throw DuplicateIdentifierException(_);
	}
	ids.add(nam, this);
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
