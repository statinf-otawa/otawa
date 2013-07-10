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

#include <otawa/prop/info.h>

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
static class Init {
public:
	Init(void): init(false) {
		init.startup();
	}

	genstruct::HashTable<String, AbstractIdentifier *> map;
	Initializer<AbstractIdentifier> init;
} ids;


/**
 * @class AbstractIdentifier
 * Represents a unique identifier used by the annotation system.
 * Identifier pointers may be compared for testing equality.
 *
 * @par Information on identifiers
 *
 * To have nice display of identifier and their value, one can provide
 * to the OTAWA identifier system informations for human reading.
 * As an identifier is also a property list, one may attach properties
 * with the following identifier:
 * @ref otawa::IDENTIFIER_LABEL -- a short label to display in front of
 * 		the property value,
 * @ref otawa::IDENTIFIER_DESC -- a more precise description of the identifier.
 *
 * Both information may be set using some dedicated macros:
 * @code
 * #include <otawa/prop/info.h>
 *
 * Identifier<int> MY_ID("MY_ID", 111);
 * SET_LABEL(MY_ID, "my identifier");
 * SET_DESC(MY_ID, "this is my identifier !");
 * @endcode
 *
 * @ingroup prop
 */


/**
 * Find an abstract identifier by its name.
 * @param name	Name of the looked identifier.
 * @return	Found identifier or null.
 */
AbstractIdentifier *AbstractIdentifier::find(const string& name) {
	return ids.map.get(name, 0);
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
AbstractIdentifier::AbstractIdentifier(cstring name)
:	nam(name)
{
	//ASSERT(!((((unsigned int)this) > 0xbf000000) && (((unsigned int)this) < 0xc0000000)));
	TRACE("construct(" << (void *)this << ", " << nam << ")");
	if(name) {
		TRACE("record(" << (void *)this << ", " << nam << ")");
		ids.init.record(this);
	}
}


/**
 * Build an identifier with the given name and properties read from
 * the given arguments that must be a list of @ref Property * pointer
 * (1) ended by a null identifier pointer and (2) and will be released
 * by the current identifier.
 * @param name		Name of the identifier ("" for anonymous)
 * @param prop		First property.
 * @param args		Arguments to derive identifier properties.
 */
AbstractIdentifier::AbstractIdentifier(cstring name, Property *prop, VarArg& args)
: nam(name) {
	initProps(prop, args);
}


/**
 * Add the properties found in the arguments (list of
 * @ref Property * ended by null) to the identifier properties.
 * @param prop	First property.
 * @param args	Null-ended properties to scan.
 */
void AbstractIdentifier::initProps(Property *prop, VarArg& args) {
	while(prop) {
		addProp(prop);
		prop = args.next<Property *>();
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
	out << "ID(";
	if(nam)
		out << nam;
	else
		out << (void *)this;
	out << ')';
}


/**
 * Print the value of the given property (accordint the property matches
 * the given identifier). It is an error to call this method with a property
 * matching a different identifier.
 *
 * Use the print() method of a property instead.
 */
void AbstractIdentifier::print(io::Output& out, const Property *prop) const {
	out << "<not printable>";
}


/**
 * Print the value of the given property in a formatted way, that is, possibly
 * to perform re-scanning after. For example, string are quoted and special
 * character escaped.
 * @param out	Output to use.
 * @param prop	Property to display.
 */
void AbstractIdentifier::printFormatted(io::Output& out, const Property *prop) const {
	out << "<not printable>";
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
	if(ids.map.get(nam)) {
		cerr << "FATAL ERROR: identifier \"" << nam << "\" defined multiple times.";
		String _(nam);
		throw DuplicateIdentifierException(_);
	}
	ids.map.add(nam, this);
}


// Specialisation for types
template <>
#if defined(__unix) || defined(__APPLE__)
const Type& Identifier<bool>::type(void) const { return Type::bool_type; }
#elif defined(__WIN32) || defined(__WIN64)
inline const Type& Identifier<bool>::type(void) const { return Type::bool_type; }
#endif

template <>
#if defined(__unix) || defined(__APPLE__)
const Type& Identifier<char>::type(void) const { return Type::char_type; }
#elif defined(__WIN32) || defined(__WIN64)
inline const Type& Identifier<char>::type(void) const { return Type::char_type; }
#endif

template <>
#if defined(__unix) || defined(__APPLE__)
const Type& Identifier<int>::type(void) const { return Type::int32_type; }
#elif defined(__WIN32) || defined(__WIN64)
inline const Type& Identifier<int>::type(void) const { return Type::int32_type; }
#endif

template <>
#if defined(__unix) || defined(__APPLE__)
const Type& Identifier<long long>::type(void) const { return Type::int64_type; }
#elif defined(__WIN32) || defined(__WIN64)
inline const Type& Identifier<long long>::type(void) const { return Type::int64_type; }
#endif

template <>
#if defined(__unix) || defined(__APPLE__)
const Type& Identifier<char *>::type(void) const { return Type::cstring_type; }
#elif defined(__WIN32) || defined(__WIN64)
inline const Type& Identifier<char *>::type(void) const { return Type::cstring_type; }
#endif


/**
 * Property of this identifier is put on identifier to give a more
 * human-readable name to the identifier.
 *
 * @par Hooks
 * @li @ref AbstractIdentifier
 * @li @ref Identifier
 *
 * @ingroup prop
 */
Identifier<cstring> IDENTIFIER_LABEL("otawa::IDENTIFIER_LABEL", "");


/**
 * Property of this identifier is put on identifier to give a more
 * precise description of the identifier.
 *
 * @par Hooks
 * @li @ref AbstractIdentifier
 * @li @ref Identifier
 *
 * @ingroup prop
 */
Identifier<cstring> IDENTIFIER_DESC("otawa::IDENTIFIER_DESC", "");


/**
 * Test if two properties are equal.
 * @param prop1		First property.
 * @param prop2		Second property.
 * @return			True if they are equal, false else.
 */
bool AbstractIdentifier::equals(const Property *prop1, const Property *prop2) const {
	return prop1 == prop2;
}


/**
 * Build a copy of the given property.
 * @param prop	Property to copy.
 * @return		Copy of the property.
 */
Property *AbstractIdentifier::copy(Property& prop) const {
	return 0;
}


/**
 * @fn Property *AbstractIdentifier::copy(Property *prop);
 * Build a copy of the given property.
 * @param prop	Property to copy.
 * @return		Copy of the property.
 */


/**
 * Serialize the workspace to the current serializer.
 * In the default implementation, do nothing.
 * @param prop			Property containing the value to serialize.
 * @param serializer	Serializer to serialize to.
 */
void AbstractIdentifier::serialize(Property *prop, elm::serial2::Serializer& serializer) {
}


/**
 * Unserialize the workspace to the current unserializer.
 * In the default implementation, do nothing.
 * @param unserializer	Unserializer to unserialize to.
 * @return				Property result of unserialization, null if the property cannot be unserialized.
 *
 */
Property * AbstractIdentifier::unserialize(elm::serial2::Unserializer& unserializer) {
	return 0;
}

} // otawa
