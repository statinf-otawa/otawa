/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	PropList class implementation
 */

#include <elm/io.h>
#include <elm/util/VarArg.h>
#include <otawa/properties.h>
using namespace elm;

namespace otawa {

/**
 * This identifier is used for marking the end of property list definition
 * in variable arguments "...".
 * @ingroup prop
 */
const AbstractIdentifier END("end", otawa::NS);


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
void PropList::init(const AbstractIdentifier *id, elm::VarArg& args) {
	while(id != &END) {
		id->scan(*this, args);
		id = args.next<const AbstractIdentifier *>();
	}
}


/**
 * Property list constructor using a sequence of (identifier, value) pairs
 * passed as variable arguments. This list must be ended by an @ref otawa::END
 * identifier.
 * @param id	First identifier.
 * @param ...	Remaining of the sequence.
 */
PropList::PropList(const AbstractIdentifier *id, ...): head(0) {
	VARARG_BEGIN(args, id)
		init(id, args);
	VARARG_END
}


/**
 * Property list constructor using a sequence of (identifier, value) pairs
 * passed as variable arguments. This list must be ended by an @ref otawa::END
 * identifier.
 * @param id	First identifier.
 * @param args	Remaining of the sequence.
 */
PropList::PropList(const AbstractIdentifier *id, elm::VarArg& args): head(0) {
	init(id, args);
}


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
	return getDeep(id);
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
 * This protected method is called when the usual property retrieval method fails. It lets a chance
 * for sub-classes to provide special ways for getting properties. It may be used, for example,
 * for implementing lazy evaluation of special values or for merging property list of many objects.
 * @param id	Identifier of the looked property.
 * @return	Return the found property or null.
 */
Property *PropList::getDeep(const AbstractIdentifier *id) const {
	return 0;
}


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
	out << "{ " << io::endl;
	for(Iter prop(this); prop; prop++)
		out << *prop << io::endl;
	out << " }";
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

