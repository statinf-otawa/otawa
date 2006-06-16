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
 * @class Property
 * A property associates a data with an identifier. They are stored in ProgObject
 * and allow storage and retrieval of any data. For efficiency purpose, they use
 * integer identifier that are associated with string names.
 */


/**
 * Allocate a new identifier matching the given name. Note that, if an identifier
 * exists with the same name, its code will be returned ensuring consistency
 * between name and code for identifiers.
 * @param name	Name of the identifier.
 * @return				Allocated identifier code.
 */
const Identifier *Property::getID(CString name) {
	return Identifier::getID(name);
}


/**
 * @fn Property::~Property(void)
 * Virtual destructor for allowing destruction of any stored data.
 */
 
/**
 * @fn Property::Property(const Identifier *_id)
 * Build a new property with the given identifier.
 * @param _id	Identifier of the property.
 */


/**
 * @fn Property::Property(CString name)
 * Build a new property with the given identifier name. The matching identifier
 * code is automatically retrieved.
 */


/**
 * @fn Property::Property(const Identifier& _id);
 * build a property from a static identifier.
 * @param _id	Property identifier.
 */


/**
 * @fn const Identifier *Property::getID(void) const
 * Get the identifier code of the property.
 * @return	Identifier code.
 */


/**
 * @fn Property *Property::getNext()
 * Get the next property.
 * @return Next property.
 */


/**
 * @fn Property *Property::copy(void);
 * This method is called when a property is copied. It may be specialized
 * by Property class children.
 * @return	Copy of the current property.
 */


/**
 * @class GenericProperty
 * This generic class allows attaching any type of data to a property.
 */


/**
 * @fn GenericProperty::GenericProperty(const Identifier *id, T _value)
 * Build a new generic property with the given value.
 * @param id		Identifier code of the property.
 * @param _value	Value of the property.
 */


/**
 * @fn GenericProperty::GenericProperty(CString name, T _value)
 * Build a new generic property by its identifier name. The matching identifier
 * code is automatically retrieved.
 * @param name		Identifier name of the property.
 * @param _value	Value of the property.
 */


/**
 * @fn GenericProperty::GenericProperty(const Identifier& id, T _value);
 * Build a generic property with a static identifier and a value.
 * @param id		Property identifier.
 * @param _value	Property value.
 */


/**
 * @fn T& GenericProperty::getValue(void) 
 * Get the value of the property.
 * @return	Value of the property.
 */


/**
 * @fn GenericProperty<T> *GenericProperty::make(const Identifier *id, const T value);
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
 */


/**
 * @fn LockedProperty<T>::LockedProperty(const Identifier *id, T _value);
 * Build a new locked property.
 * @param id		Property identifier.
 * @param _value	Lock pointer value.
 */


/**
 * @fn LockedProperty<T>::LockedProperty(const Identifier& id, T _value);
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

