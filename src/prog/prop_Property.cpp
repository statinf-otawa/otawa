/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	prog/prop_PropList.cpp -- implementation of Property class and derived.
 */

#include <elm/io.h>
#include <otawa/properties.h>
using namespace elm;

namespace otawa {

/**
 * @class GenericProperty
 * This generic class allows attaching any type of data to a property.
 * @param T	Type of the date stored in the property.
 * @ingroup prop
 */


/**
 * @fn GenericProperty::GenericProperty(const AbstractIdentifier *id, T _value)
 * Build a new generic property with the given value.
 * @param id		Identifier code of the property.
 * @param _value	Value of the property.
 */


/**
 * @fn GenericProperty::GenericProperty(elm::CString name, T _value)
 * Build a new generic property by its identifier name. The matching identifier
 * code is automatically retrieved.
 * @param name		Identifier name of the property.
 * @param _value	Value of the property.
 */


/**
 * @fn GenericProperty::GenericProperty(const AbstractIdentifier& id, T _value);
 * Build a generic property with a static identifier and a value.
 * @param id		Property identifier.
 * @param _value	Property value.
 */


/**
 * @fn T& GenericProperty::value(void);
 * Get the value of the property.
 * @return	Value of the property.
 */


/**
 * @fn const T& GenericProperty::value(void) const;
 * Get the value of the property.
 * @return	Value of the property.
 */


/**
 * @fn GenericProperty<T> *GenericProperty::make(const AbstractIdentifier *id, const T value);
 * Build a new generic property with the given data. Defining the constructor as is, allows
 * replacing the default building behaviour by specialized ones.
 * @param id		Identifier of the property.
 * @param value		Value of the property.
 * @return			Built property.
 */


/**
 * @class LockedProperty
 * This class is used for building a lock property, that is, for taking pointer
 * values implementing the elm::Locked class. It provides management of the lock
 * along the life of the property: creation, copy and deletion.
 * @param T	Type of the date stored in the property.
 * @ingroup prop
 */


/**
 * @fn LockedProperty<T>::LockedProperty(const AbstractIdentifier *id, T _value);
 * Build a new locked property.
 * @param id		Property identifier.
 * @param _value	Lock pointer value.
 */


/**
 * @fn LockedProperty<T>::LockedProperty(const AbstractIdentifier& id, T _value);
 * Build a new locked property with a static identifier.
 * @param id		Property identifier.
 * @param _value	Lock pointer value.
 */


/**
 * @fn LockedProperty<T>::LockedProperty(elm::CString name, T _value);
 * Build a new locked property with a named identifier.
 * @param name		Property identifier name.
 * @param _value	Lock pointer value.
 */

} // otawa

