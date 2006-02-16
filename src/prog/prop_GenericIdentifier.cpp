/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prog/GenericIdentifier.cpp -- implementation of GenericIdentifier class.
 */

#include <otawa/prop/GenericIdentifier.h>

namespace otawa {

/**
 * @class GenericIdentifier<T>
 * This class represents identifier with a typed associated value.
 * Identifier with a value should declared with this kind of identifier because
 * it procides full support for reflexive facilities.
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
 * @fn void GenericIdentifier<T>::add(PropList& list, const T& value);
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void GenericIdentifier<T>::set(PropList& list, const T& value);
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn elm::Option<T> GenericIdentifier<T>::get(PropList& list);
 * Get the value associated with a property matching the current identifier.
 * @param list	List to look in.
 * @return	Optional value.
 */


/**
 * @fn T GenericIdentifier<T>::get(PropList& list, const T& def);
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn T GenericIdentifier<T>::use(PropList& list);
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */
	

/**
 * @fn void GenericIdentifier<T>::add(PropList& list, const T *value);
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void GenericIdentifier<T>::set(PropList& list, const T *value);
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn elm::Option<T> GenericIdentifier<T>::get(PropList *list);
 * Get the value associated with a property matching the current identifier.
 * @param list	List to look in.
 * @return	Optional value.
 */


/**
 * @fn T GenericIdentifier<T>::get(PropList *list, const T& def);
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn T GenericIdentifier<T>::use(PropList *list);
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */

}	// otawa
