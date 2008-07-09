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
 * @defgroup prop Properties System
 * Most OTAWA objects provides a facility to store annotations. These annotations
 * are implemented using @ref otawa::Property to implement annotations and
 * @ref PropList to implement objects supporting annotations. As, in OTAWA,
 * the annotation system is the first-class feature to provide adaptability and
 * extensibility, the C++ API provides a lot of facilities to use them.
 * 
 * @section prop_what What Is An Annotation?
 * In OTAWA, annotation is a pair whose first member is an identifier and the
 * second one is value. The C++ template facilities are used to implemented
 * such a flexible feature.
 *
 * As the annotations are melted together in a same list, the identifier
 * provides a way to retrieve them from a list. They are implemented using 
 * the @ref otawa::Identifier class. In order to achieve uniqueness
 * of identifiers, they must be declared as static variables with a string
 * identifying them. During run-time, OTAWA only accepts one identifier
 * matching a name string. This constraint makes easier and faster the
 * management of identifier and, specialy, the comparison that resumes to
 * a pointer comparison.
 *
 * An identifier may be declared as a global variable or a static class member.
 * In the header file, they may be ddeclared as below:
 * @code
 * #include <otawa/otawa.h>
 * extern const Identifier MY_ID;
 * class C {
 * 		static const Identifier YOUR_ID;
 * };
 * @endcode
 * While, in the source file, they then must be defined:
 * @code
 * #include "my_header.h"
 * const Identifier MY_ID("my_id");
 * const Identifier C::YOUR_ID("your_id");
 * @endcode
 * Then, they may be used easily by giving their name in the @ref Property
 * and @ref PropList methods:
 * @code
 * int value = props.get<int>(MY_ID);
 * @endcode
 * 
 * @section prop_simple Simple Use of Annotations
 * As any other property attached to an object, the annotation may be read or
 * written. They works quitely like members of classes bu their dynamic and
 * generic behaviour requires accessing specific methods in the property list.
 * 
 * For setting a property in a property list, one may use the set() method that
 * takes the type of the value to set (often optional), the identifier to use
 * and the value to set. If the property list contains already a property with
 * this identifier, it is removed.
 * @code
 * props.set<bool>(C::YOUR_ID, true);
 * props.set(MY_ID, 1.5);
 * @endcode
 * OTAWA allows also to have many properties with the same identifier in a
 * property list. The add() method must be used in this case:
 * @code
 * props.add(C::YOUR_ID, true);
 * props.add<double>(MY_ID, 1.5);
 * @endcode
 * 
 * There are may different methods to read a property from a property list
 * taking in account the presence of the property in the list. The first form
 * of get() method takes the type of the result and the identifier of the
 * looked property. It returns an @ref elm::Option value that may contain
 * or not the property value.
 * @code
 * elm::Option<double> result = props.get<double>(MY_ID);
 * if(result)
 * 	// Property found
 * else
 * 	// Property not found
 * @endcode
 * In the second form, the get() takes also a value that is the default returned
 * value if the property is not found.
 * @code
 * bool result = props.get<bool>(C::YOUR_ID, false);
 * @endcode
 * Finally, the use() method may speed up the access when the property is known
 * to be in the list. Notice that an "assertion failure" is thrown if the
 * property is not found!
 * @code
 * bool result = props.use<bool>(C::YOUR_ID);
 * @endcode
 * 
 * @section prop_iter Iterator Access
 * As most object collection in OTAWA, the properties may be visited using
 * iterators with the class Property::PropIter.
 * @code
 * for(PropList::PropIter prop(props); prop; prop++) {
 * 	cout << "identifier = " << prop->id() << endl;
 * 	cout << "value = " << prop.get<int>() << endl;
 * }
 * @endcode
 * 
 * As the use of the PropList::add() method may create many properties with
 * the same identifier, a specific iterator is provided to visit the instances
 * of a same identifier: PropList::PropGetter.
 * @code
 * for(PropList::PropGetter prop(props, MY_ID); prop; prop++)
 * 	cout << "value = " << prop.get<double>() << endl;
 * @endcode
 * 
 * @section prop_gen Generic Annotation
 * Yet, as re-typing at each use the type of annotation value may be painful
 * and also error-prone (there is no type checking between the property
 * creation and the retrieval), OTAWA introduced otawa::GenericIdentifier
 * identifier that embed the type of the associated value. Such an identifier
 * is declared is below:
 * @code
 * Header File
 * GenericIdentifier<int> AN_ID;
 * 
 * Source File
 * GenericIdentifier<int> AN_ID("an_id");
 * @endcode
 * 
 * The, they provides a functional-like notation to read and write the value:
 * @code
 * int value = AN_ID(props);
 * int value = AN_ID(props, 111);
 * AN_ID(props) = 111;
 * @endcode
 * It is currently the preferred form to use properties but a lot of already-
 * existing identifiers in OTAWA does not already supports this work.
 */

/**
 * @class Property
 * A property associates a data with an identifier. They are stored in ProgObject
 * and allow storage and retrieval of any data. For efficiency purpose, they use
 * integer identifier that are associated with string names.
 * @ingroup prop
 */


/**
 * Allocate a new identifier matching the given name. Note that, if an identifier
 * exists with the same name, its code will be returned ensuring consistency
 * between name and code for identifiers.
 * @param name	Name of the identifier.
 * @return				Allocated identifier code.
 */
const AbstractIdentifier *Property::getID(elm::CString name) {
	return AbstractIdentifier::find(name);
}


/**
 * @fn Property::~Property(void)
 * Virtual destructor for allowing destruction of any stored data.
 */


/**
 * @fn Property::Property(const AbstractIdentifier *_id)
 * Build a new property with the given identifier.
 * @param _id	Identifier of the property.
 */


/**
 * @fn Property::Property(const AbstractIdentifier& _id);
 * build a property from a static identifier.
 * @param _id	Property identifier.
 */


/**
 * @fn Property::Property (elm::CString name);
 * Build a property with the name of its identifier.
 * @param name	Name of the identifier.
 */


/**
 * @fn const Identifier *Property::id(void) const
 * Get the identifier code of the property.
 * @return	Identifier code.
 */


/**
 * @fn Property *Property::next()
 * Get the next property.
 * @return Next property.
 */


/**
 * Print the given property, that is, the identifier and its value if any.
 * @param out	Output to use.
 */
void Property::print(elm::io::Output& out) const {
	out << _id << " = ";
	_id->print(out, this);
}


/**
 * @fn Property *Property::copy(void);
 * This method is called when a property is copied. It may be specialized
 * by Property class children.
 * @return	Copy of the current property.
 */


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

