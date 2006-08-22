/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prog/GenericIdentifier.cpp -- implementation of GenericIdentifier class.
 */

#include <otawa/prop/GenericIdentifier.h>
#include <otawa/prop/PropList.h>

namespace otawa {

/**
 * @class GenericIdentifier otawa/properties.h
 * This class represents identifier with a typed associated value.
 * Identifier with a value should declared with this kind of identifier because
 * it procides full support for reflexive facilities.
 * @param T	The type of data stored in a property with this identifier.
 * @p Note that the property management functions of this class are safer to
 * use than the rough @ref PropList functions because they ensure safe value
 * type management.
 */


/**
 * @fn GenericIdentifier<T>::GenericIdentifier(elm::CString name);
 * Build a new generic identifier.
 * @param name	Name of the generic identifier.
 */


/**
 * @fn GenericIdentifier<T>::GenericIdentifier(elm::CString name, const T& default_value);
 * Build a new generic identifier.
 * @param name			Name of the generic identifier.
 * @param default_value	Default value of the identifier.
 */


/**
 * @fn void GenericIdentifier<T>::add(PropList& list, const T& value);
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */



/**
 * @fn void GenericIdentifier<T>::add(PropList& list, const T& value) const;
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void GenericIdentifier<T>::add(PropList *list, const T& value) const;
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void GenericIdentifier<T>::set(PropList& list, const T& value) const;
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn void GenericIdentifier<T>::set(PropList *list, const T& value) const;
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn const T& GenericIdentifier<T>::get(const PropList& list, const T& def) const;
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn const T& GenericIdentifier<T>::get(const PropList *list, const T& def) const;
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn const T& GenericIdentifier<T>::use(const PropList& list) const;
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */


/**
 * @fn const T& GenericIdentifier<T>::use(const PropList *list) const;
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */



/**
 * @fn const T& GenericIdentifier<T>::value(const PropList& list) const;
 * For internal use only.
 * @internal Same as get() without default value. Only provided for symmetry.
 */


/**
 * @fn const T& GenericIdentifier<T>::value(const PropList *list) const;
 * For internal use only.
 * @internal Same as get() without default value. Only provided for symmetry.
 */


/**
 * @fn Value GenericIdentifier<T>::value(PropList& list) const;
 * For internal use only.
 * @internal Provide an assignable value.
 */


/**
 * @fn Value GenericIdentifier<T>::value(PropList *list) const;
 * For internal use only.
 * @internal Provide an assignable value.
 */


/**
 * @fn const T& GenericIdentifier<T>::operator()(const PropList& props) const;
 * Read the value in a functional way.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn const T& GenericIdentifier<T>::operator()(const PropList *props) const;
 * Read the value in a functional way.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn Value GenericIdentifier<T>::operator()(PropList& props) const;
 * Read or write a property value in a functional way. The returned value
 * may be read (automatic conversion to the value) or written (using operator
 * = to set the value or += to add a new value at the property list.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn Value GenericIdentifier<T>::operator()(PropList *props) const;
 * Read or write a property value in a functional way. The returned value
 * may be read (automatic conversion to the value) or written (using operator
 * = to set the value or += to add a new value at the property list.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


// GenericIdentifier<T>::print Specializations
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


template <>
void GenericIdentifier<char>::print(elm::io::Output& out, const Property& prop) const {
	out << '\'';
	escape(out, ((const GenericProperty<char> &)prop).value(), '\'');
	out << '\'';
}


template <>
void GenericIdentifier<CString>::print(elm::io::Output& out, const Property& prop) const {
	out << '"';
	CString str = ((const GenericProperty<CString> &)prop).value();
	for(int i = 0; str[i]; i++)
		escape(out, str[i], '"'); 
	out << '"';
}


template <>
void GenericIdentifier<elm::String>::print(elm::io::Output& out, const Property& prop) const {
	out << '"';
	const String& str = ((const GenericProperty<String> &)prop).value();
	for(int i = 0; i < str.length(); i++)
		escape(out, str[i], '"'); 
	out << '"';
}


template <>
void GenericIdentifier<PropList *>::print(elm::io::Output& out, const Property& prop) const {
	out << "proplist(" << &prop << ")";
}


// GenericIdentifier<T>::scan Specializations
template <>
void GenericIdentifier<CString>::scan(PropList& props, VarArg& args) const {
	props.set(*this, args.next<char *>());
}


template <>
void GenericIdentifier<String>::scan(PropList& props, VarArg& args) const {
	props.set(*this, args.next<char *>());
}

}	// otawa
