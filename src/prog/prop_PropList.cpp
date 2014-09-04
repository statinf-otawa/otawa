/*
 *	PropList class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-8, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <elm/io.h>
#include <elm/util/VarArg.h>
#include <otawa/properties.h>
using namespace elm;

namespace otawa {

/**
 * @defgroup prop Properties System
 *
 * The properties are the first-class citizens of the OTAWA core used to annotate
 * a program representation. They form a simple and usable facility to attach
 * to/retrieve from the program representation some pieces of information.
 *
 *
 * @section prop_def Principles
 *
 * Most OTAWA objects provides a facility to store annotations. These annotations
 * are implemented using @ref otawa::Property to implement annotations and
 * @ref PropList to implement objects supporting annotations. As, in OTAWA,
 * the annotation system is the first-class feature to provide adaptability and
 * extensibility, the C++ API provides a lot of facilities to use them.
 *
 * An annotation is a pair whose first member is an identifier and the
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
 * @section prop_using Using the properties
 *
 * A property is composed of:
 * @li an identifier (usually an object of the class ''otawa::Identifier''),
 * @li the type of the stored data,
 * @li the stored data itself.
 *
 * The properties work as dynamic fields of the object they are hooked to.
 * As C++ does not support such a kind of field, OTAWA encapsulates the management of
 * properties in a specific syntax based on the C++ operator overload ability. So,
 * the syntax to read a property the identifier of which is ''ID'', attached
 * to the object ''list'' (an object that has some properties attached to it is called
 * a property list) is:
 * @code
 * ID ( list )
 * @endcode
 *
 * To set a property, one has just to use the same syntax followed by the ''='' equal
 * assignment symbol and the assigned expression. If the annotation is already hooked
 * to the property list, its value is easily replaced.
 * @code
 * ID ( list ) = expression ;
 * @endcode
 *
 * The property system allow hooking several properties with the same identifier to a list.
 * The trick is easily performed using the ''add()'' method.
 * @code
 * ID ( list ).add( expression );
 * @endcode
 *
 * To retrieve properties with the same identifier, one has to use a clumsy syntax as below:

  for(Identifier< data type > data( list , ID ); data; data++)
    use(data);

The properties may also be removed using the ''remove'' method of the ''Identifier'' class.

  ID ( list ) . remove ( ) ;

Although the access time to OTAWA properties is longer than an access to classical C++ fields, the penalty is reduced thanks to a cache system that benefits from the temporal locality of accesses. The properties also have a slightly larger size in memory. Yet, these drawbacks are balanced by the induced improvement in flexibility and usability to work on the program representation.

This section has listed the main primitives used to handle properties. The following section will show how to declare ''Identifier'' objects.
 *
 *
 *
 *
 *
 *
 *
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
 * @fn Property::~Property(void)
 * Virtual destructor for allowing destruction of any stored data.
 */


/**
 * @fn Property *Property::copy(void);
 * This method is called when a property is copied. It may be specialized
 * by Property class children.
 * @return	Copy of the current property.
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
 * This identifier is used for marking the end of property list definition
 * in variable arguments "...".
 * @ingroup prop
 */
const AbstractIdentifier END("end");


/**
 * @class PropList
 * This a list of properties. This may be inherited for binding properties to
 * other classes or used as-is for passing heterogeneous properties to a function
 * call.
 * @ingroup prop
 */


/**
 * Initialize a property list from a sequence of (identifier, value) pairs
 * passed as variable arguments. This list must be ended by an @ref otawa::END
 * identifier.
 * @param id	First identifier.
 * @param args	Variable arguments.
 */
/*void PropList::init(const AbstractIdentifier *id, elm::VarArg& args) {
	while(id != &END) {
		id->scan(*this, args);
		id = args.next<const AbstractIdentifier *>();
	}
}*/


/**
 * @fn PropList::PropList(const PropList& props);
 * Build a property list as a copy of another one.
 * @param props	Property list to copy.
 */


/**
 * @fn PropList::PropList(void): head(0);
 * Build an empty property list.
 */


/**
 * Add all properties from the given property list, in a reverse order.
 * @param props	Property list to clone.
 */
void PropList::addProps(const PropList& props) {
	for(Property *cur = props.head; cur; cur = cur->next()) {
		Property *copy = cur->copy();
		addProp(copy);
	}
}


/**
 * Find a property by its identifier.
 * @param id	Identifier of the property to find.
 * @return		Found property or null.
 */
Property *PropList::getProp(const AbstractIdentifier *id) const {

	/* Look in this list */
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next())
		if(cur->id() == id) {
			if(prev) {
				prev->_next = cur->next();
				cur->_next = head;
				head = cur;
			}
			return cur;
		}

	/* Perform deep search */
	return 0;
}


/**
 * Set the property in the property list removing any double..
 * @param prop	Property to set.
 */
void PropList::setProp(Property *prop) {

	// Find the property
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next())
		if(cur->id() == prop->id()) {
			if(prev)
				prev->_next = cur->next();
			else
				head = cur->next();
			delete cur;
			break;
		}

	// Link the new property
	prop->_next = head;
	head = prop;
}


/**
 * @fn void PropList::setProp(const Identifier *id);
 * Set an indicator property (without value).
 * @param id	Identifier of the property.
 */


/**
 * Remove a property matching the given identifier.
 * @param id	Identifier of the property to remove.
 */
void PropList::removeProp(const AbstractIdentifier *id) {
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next())
		if(cur->id() == id) {
			if(prev)
				prev->_next = cur->next();
			else
				head = cur->next();
			delete cur;
			break;
		}
}


/**
 * Remove a property matching the given identifier and return it.
 * Caller is responsible of the management of the obtained property.
 * @param id	Identifier of the property to extract.
 */
Property *PropList::extractProp(const AbstractIdentifier *id) {
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next())
		if(cur->id() == id) {
			if(prev)
				prev->_next = cur->next();
			else
				head = cur->next();
			return cur;
			break;
		}
	return 0;
}


/**
 * @fn Property *PropList::extractProp(const AbstractIdentifier& id);
 * Remove a property matching the given identifier and return it.
 * Caller is responsible of the management of the obtained property.
 * @param id	Identifier of the property to extract.
 */


/**
 * Remove all properties from the list.
 */
void PropList::clearProps(void) {
	for(Property *cur = head, *next; cur; cur = next) {
		next = cur->next();
		delete cur;
	}
	head = 0;
}


/**
 * @fn T PropList::get(const AbstractIdentifier *id, T def_value) const;
 * Get the value of a property.
 * @param id			Identifier of the property to get.
 * @param def_value		Default value returned if property does not exists.
 * @return				Value of the property.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn T PropList::get(const AbstractIdentifier& id, T def_value) const;
 * Get the value of a property.
 * @param id			Identifier of the property to get.
 * @param def_value		Default value returned if property does not exists.
 * @return				Value of the property.
 */


/**
 * @fn Option<T> PropList::get(const AbstractIdentifier *id) const;
 * Get the value of a property.
 * @param id	Identifier of the property to get the value from.
 * @return		None or the value of the property.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn Option<T> PropList::get(const AbstractIdentifier& id) const;
 * Get the value of a property.
 * @param id	Identifier of the property to get the value from.
 * @return		None or the value of the property.
 */


/**
 * @fn T& PropList::use(const AbstractIdentifier *id) const;
 * Get the reference on the value of the given property. If not found,
 * cause an assertion failure.
 * @param id	Identifier of the property to get the value from.
 * @return		Reference on the property value.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn T& PropList::use(const AbstractIdentifier& id) const;
 * Get the reference on the value of the given property. If not found,
 * cause an assertion failure.
 * @param id	Identifier of the property to get the value from.
 * @return		Reference on the property value.
 */


/**
 * @fn void PropList::set(const AbstractIdentifier *id, const T value)
 * Set the value of a property.
 * @param id	Identifier of the property.
 * @param value	Value of the property.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn void PropList::set(const AbstractIdentifier& id, const T value)
 * Set the value of a property.
 * @param id	Identifier of the property.
 * @param value	Value of the property.
 */


/**
 * Add property to the list without checking of duplication.
 * @param prop	Property to add.
 */
void PropList::addProp(Property *prop) {
	prop->_next = head;
	head = prop;
}


/**
 * Remove all the properties matching the given identifier.
 * @param id	Identifier of properties to remove.
 */
void PropList::removeAllProp(const AbstractIdentifier *id) {
	Property *prv = 0, *cur = head;
	while(cur) {
		if(cur->id() != id) {
			prv = cur;
			cur = cur->next();
		}
		else {
			if(prv) {
				prv->_next = cur->next();
				delete cur;
				cur = prv->next();
			}
			else {
				head = cur->next();
				delete cur;
				cur = head;
			}
		}
	}
}


/**
 * @fn void PropList::add(const AbstractIdentifier *id, const T value);
 * Add a ne wproperty to the property list without replacing a possible
 * existing one.
 * @param id		Property identifier.
 * @param value		Property value.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn void PropList::add(const AbstractIdentifier& id, const T value);
 * Add a ne wproperty to the property list without replacing a possible
 * existing one.
 * @param id		Property identifier.
 * @param value		Property value.
 */


/**
 * @fn void PropList::addLocked(const AbstractIdentifier *id, const T value);
 * Add a locked property to the property list. A locked property value inherits
 * from Elm::Locked class and provides a lock that will release the value when
 * there is no more lock. This kind of property provides the ability to
 * manage the lock.
 * @param id		Property identifier.
 * @param value		Property value.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn void PropList::addLocked(const AbstractIdentifier& id, const T value);
 * Add a locked property to the property list. A locked property value inherits
 * from Elm::Locked class and provides a lock that will release the value when
 * there is no more lock. This kind of property provides the ability to
 * manage the lock.
 * @param id		Property identifier.
 * @param value		Property value.
 */


/**
 * @fn void PropList::addDeletable(const AbstractIdentifier& id, const T value);
 * Add an annotation with a deletable value, that is, a pointer that  will be
 * deleted when the annotation is destroyed.
 * @param id	Annotation identifier.
 * @param value	Annotation value.
 */


/**
 * This is an empty proplist for convenience.
 */
const PropList PropList::EMPTY;


/**
 * @fn bool PropList::hasProp(const AbstractIdentifier& id) const;
 * Test if the property list contains a property matching the given identifier.
 * @param id	Property identifier to look for.
 * @return		True if the list contains the matching property, false else.
 */


/**
 * Display the current property list.
 * @param out	Output to use.
 */
void PropList::print(elm::io::Output& out) const {
	if(!head)
		out << "{ }";
	else {
		bool first = true;
		out << "{ ";
		for(Iter prop(this); prop; prop++) {
			if(first)
				first = false;
			else
				out << ", ";
			prop->print(out);
		}
		out << " }";
	}
}


/**
 * @class PropList::Iter
 * This iterator is used for reading all properties of a property list.
 * @ingroup prop
 */


/**
 * @fn PropList::Iter::Iter(const PropList& list);
 * Build a property iterator.
 * @param list	Property list to traverse.
 */


/**
 * @fn PropList::Iter::Iter(const PropList *list);
 * Build a property iterator.
 * @param list	Property list to traverse.
 */


/**
 * @fn void PropList::Iter::next(void);
 * Go to the next property.
 */


/**
 * @fn bool PropList::Iter::ended(void) const;
 * Test if there is still a property to examine.
 * @return	True if traversal is ended.
 */


/**
 * @fn Property *PropList::Iter::item(void) const;
 * Get the current property.
 */


/**
 * @fn Property *PropList::Iter::get(void) const;
 * Get the current property.
 */


/**
 * @fn PropList::Iter::operator bool(void) const;
 * Test if the traversal may continue.
 * @return	True if there is still porperties to examine.
 */


/**
 * @fn PropList::Iter::operator Property *(void) const;
 * Automatic conversion of iterator to property.
 * @return	Current property.
 */


/**
 * @fn PropList::Iter& PropList::Iter::operator++(void);
 * Shortcut to next() method call.
 */


/**
 * @fn Property *PropList::Iter::operator->(void) const;
 * Shortcut for accessing a member of the current property.
 */


/**
 * @fn bool PropList::Iter::operator==(const AbstractIdentifier *id) const;
 * Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @fn bool PropList::Iter::operator!=(const AbstractIdentifier *id) const;
 * Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @fn bool PropList::Iter::operator==(const AbstractIdentifier& id) const;
 *  Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @fn bool PropList::Iter::operator!=(const AbstractIdentifier& id) const;
 * Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @class PropList::Getter
 * This class is used for accessing all properties of property list with a
 * given identifier.
 * @ingroup prop
 */


/**
 * @fn PropList::Getter::Getter(const PropList& list, const AbstractIdentifier& _id);
 * Build an iterator on properties matching the given name.
 * @param list	Property list to traverse.
 * @param _id	Looked identifier.
 */


/**
 * @fn PropList::Getter::Getter(const PropList *list, const AbstractIdentifier& _id);
 * Build an iterator on properties matching the given name.
 * @param list	Property list to traverse.
 * @param _id	Looked identifier.
 */


/**
 * @fn void PropList::Getter::next(void);
 * Go to the next property.
 */


/**
 * @fn bool PropList::Getter::ended(void) const;
 * Test if there is still a property to examine.
 * @return	True if traversal is ended.
 */


/**
 * @fn Property *PropList::Getter::item(void) const;
 * Get the current property.
 */


/**
 * @fn PropList::Getter::operator bool(void) const;
 * Test if the traversal may continue.
 * @return	True if there is still porperties to examine.
 */


/**
 * @fn PropList::Getter::operator Property *(void) const;
 * Automatic conversion of iterator to property.
 * @return	Current property.
 */


/**
 * @fn Getter& PropList::PropFilter::operator++(void);
 * Shortcut to next() method call.
 */


/**
 * @fn Property *PropList::Getter::operator->(void) const;
 * Shortcut for accessing a member of the current property.
 */


} // otawa

