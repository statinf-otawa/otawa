/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prop_DeletableProperty.cpp -- DeletableProperty class implementation.
 */

 #include <otawa/prop/DeletableProperty.h>
 
namespace otawa {

/**
 * @class DeletableProperty
 * A deletable property is a genericproperty whose value will be deleted
 * when the property is destroyed.
 * @param T	Type of the date stored in the property.
 * @ingroup prop
 */
 

/**
 * @fn DeletableProperty::~DeletableProperty(void);
 * Delete also its value.
 */


/**
 * @fn Property *DeletableProperty::copy(void);
 * See @ref GenericProperty::copy().
 */


/**
 * @fn DeletableProperty::DeletableProperty(const Identifier *id, T _value);
 * Build a new deletable property.
 * @param id		Annotation identifier.
 * @param _value	Annotation value.
 */
 

/**
 * @fn DeletableProperty::DeletableProperty(const Identifier& id, T _value);
 * Build a new deletable property.
 * @param id		Annotation identifier.
 * @param _value	Annotation value.
 */


/**
 * @fn DeletableProperty::DeletableProperty(elm::CString name, T _value);
 * Build a new deletable property.
 * @param name		Annotation name (matching identifier is automatically
 * 					retrieved).
 * @param _value	Annotation value.
 */

}	// otawa
