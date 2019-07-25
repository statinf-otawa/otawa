/*
 * OTAWA -- WCET computation framework
 * Copyright (C) 2006-08  IRIT - UPS <casse@irit.fr>
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

#include <otawa/prop/Identifier.h>
#include <otawa/prop/PropList.h>
#include <elm/io/BlockInStream.h>
#include <elm/io/Input.h>
#include <otawa/util/SymAddress.h>

namespace otawa {

/**
 * @class Identifier
 * This class represents identifier with a typed associated value.
 * Identifier with a value should declared with this kind of identifier because
 * it provides full support for reflexive facilities.
 * @param T	The type of data stored in a property with this identifier.
 * @p Note that the property management functions of this class are safer to
 * use than the rough @ref PropList functions because they ensure safe value
 * type management.
 *
 * Identifiers provide also a way to read their argument from string using fromString() method
 * that calls, in turn, the from_string() function. from_string() is provided for most used
 * types of OTAWA and will raise an exception else. Use can customize the from_string() function
 * with their own types using the code below:
 * @code
 * namespace otawa {
 * 	class MyType {
 * 		...
 * 	};
 *
 * 	template <> void from_string(const string& s, MyType& v) {
 * 		// some work on string s to produce v
 * 	}
 * }
 * @endcode
 *
 * @ingroup prop
 */


/**
 * @fn Identifier<T>::Identifier(elm::CString name);
 * Build a new generic identifier.
 * @param name	Name of the generic identifier.
 */


/**
 * @fn Identifier<T>::Identifier(elm::CString name, const T& default_value);
 * Build a new generic identifier.
 * @param name			Name of the generic identifier.
 * @param default_value	Default value of the identifier.
 */


/**
 * @fn void Identifier<T>::add(PropList& list, const T& value) const;
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void Identifier<T>::add(PropList *list, const T& value) const;
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void Identifier<T>::set(PropList& list, const T& value) const;
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn void Identifier<T>::set(PropList *list, const T& value) const;
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn const T& Identifier<T>::get(const PropList& list, const T& def) const;
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn const T& Identifier<T>::get(const PropList *list, const T& def) const;
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn const T& Identifier<T>::use(const PropList& list) const;
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */


/**
 * @fn const T& Identifier<T>::use(const PropList *list) const;
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */



/**
 * @fn const T& Identifier<T>::value(const PropList& list) const;
 * For internal use only.
 * @internal Same as get() without default value. Only provided for symmetry.
 */


/**
 * @fn const T& Identifier<T>::value(const PropList *list) const;
 * For internal use only.
 * @internal Same as get() without default value. Only provided for symmetry.
 */


/**
 * @fn Value Identifier<T>::value(PropList& list) const;
 * For internal use only.
 * @internal Provide an assignable value.
 */


/**
 * @fn Value Identifier<T>::value(PropList *list) const;
 * For internal use only.
 * @internal Provide an assignable value.
 */


/**
 * @fn const T& Identifier<T>::operator()(const PropList& props) const;
 * Read the value in a functional way.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn const T& Identifier<T>::operator()(const PropList *props) const;
 * Read the value in a functional way.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn Value Identifier<T>::operator()(PropList& props) const;
 * Read or write a property value in a functional way. The returned value
 * may be read (automatic conversion to the value) or written (using operator
 * = to set the value or += to add a new value at the property list.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn Value Identifier<T>::operator()(PropList *props) const;
 * Read or write a property value in a functional way. The returned value
 * may be read (automatic conversion to the value) or written (using operator
 * = to set the value or += to add a new value at the property list.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


// Identifier<T>::print Specializations
static void escape(elm::io::Output& out, char chr, char quote) {
	if(chr >= ' ') {
		if(chr == quote)
			out << '\\' << quote;
		else
			out << chr;
	}
	else
		switch(chr) {
		case '\n': out << "\\n"; break;
		case '\t': out << "\\t"; break;
		case '\r': out << "\\r"; break;
		default: out << "\\x" << io::hex((unsigned char)chr); break;
	}
}


template <> void Identifier<char>::printFormatted(io::Output& out, const Property *prop) const {
	out << '\'';
	escape(out, get(*prop), '\'');
	out << '\'';
}


template <> void Identifier<string>::printFormatted(io::Output& out, const Property *prop) const {
	string str = get(*prop);
	out << '"';
	for(int i = 0; str[i]; i++)
		escape(out, str[i], '"');
	out << '"';
}


template <> void Identifier<cstring>::printFormatted(io::Output& out, const Property *prop) const {
	cstring str = get(*prop);
	out << '"';
	for(int i = 0; i < str.length(); i++)
		escape(out, str[i], '"');
	out << '"';
}


template <> void Identifier<PropList>::printFormatted(io::Output& out, const Property *prop) const {
	out << "proplist(" << &get(*prop) << ")";
}


template <> void Identifier<const PropList *>::printFormatted(io::Output& out, const Property *prop) const {
	out << "proplist(" << (void *)get(*prop) << ")";
}


// GenericIdentifier<T>::scan Specializations
/*template <>
void Identifier<CString>::scan(PropList& props, VarArg& args) const {
	cerr << "Identifier<CString>::scan()\n";
	set(props, args.next<char *>());
}*/


/*template <>
void Identifier<String>::scan(PropList& props, VarArg& args) const {
	cerr << "Identifier<String>::scan()\n";
	set(props, args.next<char *>());
}*/


/**
 * @fn void Identifier::remove(PropList& list) const;
 * Remove the property with the current identifier from the given list.
 * @param list	List to remove from.
 */


/**
 * @fn bool Identifier::exists(PropList& list) const;
 * Test if the given list contains a property with the current identifier.
 * @param list	List to look in.
 * @return		True if there is a property with current identifier, false else.
 */


/**
 * @fn void Identifier::remove(PropList *list) const;
 * Remove the property with the current identifier from the given list.
 * @param list	List to remove from.
 */


/**
 * @fn bool Identifier::exists(PropList *list) const;
 * Test if the given list contains a property with the current identifier.
 * @param list	List to look in.
 * @return		True if there is a property with current identifier, false else.
 */


/**
 * @fn Identifier<T>::fromString(PropList& props, const string& str)
 * Extract a property value from a string and store it in the given property list.
 */


// specialization of from_string
template <> void from_string(const string& str, bool& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanBool(); }

template <> void from_string(const string& str, int& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanLong(); }

template <> void from_string(const string& str, unsigned int& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanULong(); }

template <> void from_string(const string& str, long& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanLong(); }

template <> void from_string(const string& str, unsigned long& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanULong(); }

template <> void from_string(const string& str, long long& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanLLong(); }

template <> void from_string(const string& str, unsigned long long& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanULLong(); }

template <> void from_string(const string& str, double& v)
	{ io::BlockInStream stream(str); io::Input in(stream); v = in.scanDouble(); }

template <> void from_string(const string& str, string& v)
	{ v = str; }

template <> void from_string(const string& str, sys::Path& v)
	{ v = str; }

template <> void from_string(const string& str, Address& addr) {
	string buf = str;
	Address::page_t page = 0;
	int pos = buf.indexOf(':');
	if(pos >= 0) {
		io::BlockInStream stream(buf.substring(0, pos));
		io::Input in(stream);
		page = in.scanULong();
		buf = buf.substring(pos + 1);
	}
	io::BlockInStream stream(str);
	io::Input in(stream);
	Address::offset_t off = in.scanULong();
	addr = Address(page, off);
}

template <> void from_string(const string& str, SymAddress*& v) {
	v = SymAddress::parse(str);
}

void from_string_split(const string& str, Vector<string>& items) {
	int i = str.indexOf(','), j = 0;
	while(i >= 0) {
		items.add(str.substring(j, i - j));
		j = i + 1;
		i = str.indexOf(',', j);
	}
	items.add(str.substring(j));
}

}	// otawa
