/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	prog/prop_PropList.cpp -- implementation of PropList class.
 */

#include <elm/io.h>
#include <otawa/properties.h>
using namespace elm;

namespace otawa {

/**
 * @class PropList
 * This a list of properties. This may be inherited for binding properties to
 * other classes or used as-is for passing heterogeneous properties to a function
 * call.
 */


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
 * Add all properties from the given property list.
 * @param props	Property list to clone.
 */
void PropList::addProps(const PropList& props) {
	for(Property *cur = props.head; cur; cur = cur->next) {
		Property *copy = cur->copy();
		setProp(copy);
	}
}


/**
 * Find a property by its identifier.
 * @param id	Identifier of the property to find.
 * @return		Found property or null.
 */
Property *PropList::getProp(Identifier *id) {
	
	/* Look in this list */
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next)
		if(cur->id == id) {
			if(prev) {
				prev->next = cur->next;
				cur->next = head;
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
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next)
		if(cur->id == prop->id) {
			if(prev)
				prev->next = cur->next;
			else
				head = cur->next;
			delete cur;
			break;
		}
	
	// Link the new property
	prop->next = head;
	head = prop;
}


/**
 * @fn void PropList::setProp(Identifier *id);
 * Set an indicator property (without value).
 * @param id	Identifier of the property.
 */


/**
 * Remove a property matching the given identifier.
 * @param id	Identifier of the property to remove.
 */
void PropList::removeProp(Identifier *id) {
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next)
		if(cur->id == id) {
			if(prev)
				prev->next = cur->next;
			else
				head = cur->next;
			delete cur;
			break;
		}
}


/**
 * Remove all properties from the list.
 */
void PropList::clearProps(void) {
	for(Property *cur = head, *next; cur; cur = next) {
		next = cur->next;
		delete cur;
	}
	head = 0;
}


/**
 * @fn T PropList::get(Identifier *id, T def_value);
 * Get the value of a property.
 * @param id			Identifier of the property to get.
 * @param def_value		Default value returned if property does not exists.
 * @return				Value of the property.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn T PropList::get(Identifier& id, T def_value);
 * Get the value of a property.
 * @param id			Identifier of the property to get.
 * @param def_value		Default value returned if property does not exists.
 * @return				Value of the property.
 */


/**
 * @fn Option<T> PropList::get(Identifier *id)
 * Get the value of a property.
 * @param id	Identifier of the property to get the value from.
 * @return		None or the value of the property.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn Option<T> PropList::get(Identifier& id)
 * Get the value of a property.
 * @param id	Identifier of the property to get the value from.
 * @return		None or the value of the property.
 */


/**
 * @fn T& PropList::use(Identifier *id)
 * Get the reference on the value of the given property. If not found,
 * cause an assertion failure.
 * @param id	Identifier of the property to get the value from.
 * @return		Reference on the property value.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn T& PropList::use(Identifier& id)
 * Get the reference on the value of the given property. If not found,
 * cause an assertion failure.
 * @param id	Identifier of the property to get the value from.
 * @return		Reference on the property value.
 */


/**
 * @fn void PropList::set(Identifier *id, const T value)
 * Set the value of a property.
 * @param id	Identifier of the property.
 * @param value	Value of the property.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn void PropList::set(Identifier& id, const T value)
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
Property *PropList::getDeep(Identifier *id) {
	return 0;
}


/**
 * Add property to the list without checking of duplication.
 * @param prop	Property to add.
 */
void PropList::addProp(Property *prop) {
	prop->next = head;
	head = prop;
}


/**
 * Remove all the properties matching the given identifier.
 * @param id	Identifier of properties to remove.
 */
void PropList::removeAllProp(Identifier *id) {
	Property *prv = 0, *cur = head;
	while(cur) {
		if(cur->id != id) {
			prv = cur;
			cur = cur->next;
		}
		else {
			if(prv) {
				prv->next = cur->next;
				delete cur;
				cur = prv->next;
			}
			else {
				head = cur->next;
				delete cur;
				cur = head;
			}
		}
	}
}


/**
 * @fn void PropList::add(Identifier *id, const T value);
 * Add a ne wproperty to the property list without replacing a possible
 * existing one.
 * @param id		Property identifier.
 * @param value		Property value.
 * @deprecated{ Use the version using Identifier referencers.}
 */


/**
 * @fn void PropList::add(Identifier& id, const T value);
 * Add a ne wproperty to the property list without replacing a possible
 * existing one.
 * @param id		Property identifier.
 * @param value		Property value.
 */


/**
 * @fn void PropList::addLocked(Identifier *id, const T value);
 * Add a locked property to the property list. A locked property value inherits
 * from Elm::Locked class and provides a lock that will release the value when
 * there is no more lock. This kind of property provides the ability to
 * manage the lock.
 * @param id		Property identifier.
 * @param value		Property value.
 * @deprecated{ Use the version using Identifier referencers.}
 */ 


/**
 * @fn void PropList::addLocked(Identifier& id, const T value);
 * Add a locked property to the property list. A locked property value inherits
 * from Elm::Locked class and provides a lock that will release the value when
 * there is no more lock. This kind of property provides the ability to
 * manage the lock.
 * @param id		Property identifier.
 * @param value		Property value.
 */ 


/**
 * @fn void PropList::addDeletable(Identifier& id, const T value);
 * Add an annotation with a deletable value, that is, a pointer that  will be
 * deleted when the annotation is destroyed.
 * @param id	Annotation identifier.
 * @param value	Annotation value.
 */


/**
 * @class PropIter
 * This iterator is used for reading all properties of a property list.
 */


/**
 * @fn PropIter::PropIter(PropList& list);
 * Build a property iterator.
 * @param list	Property list to traverse.
 */


/**
 * @fn void PropIter::next(void);
 * Go to the next property.
 */


/**
 * @fn bool PropIter::ended(void) const;
 * Test if there is still a property to examine.
 * @return	True if traversal is ended.
 */


/**
 * @fn Property *PropIter::get(void) const;
 * Get the current property.
 */



/**
 * @fn PropIter::operator bool(void) const;
 * Test if the traversal may continue.
 * @return	True if there is still porperties to examine.
 */


/**
 * @fn PropIter::operator Property *(void) const;
 * Automatic conversion of iterator to property.
 * @return	Current property.
 */


/**
 * @fn PropIter& PropIter::operator++(void);
 * Shortcut to next() method call.
 */


/**
 * @fn Property *PropIter::operator->(void) const;
 * Shortcut for accessing a member of the current property.
 */


/**
 * @fn bool PropIter::operator==(Identifier *id) const;
 * Equality overload for testing if a property is equals to an identifier.
 */
 

/**
 * @fn bool PropIter::operator!=(Identifier *id) const;
 * Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @fn bool PropIter::operator==(Identifier& id) const;
 *  Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @fn bool PropIter::operator!=(Identifier& id) const;
 * Equality overload for testing if a property is equals to an identifier.
 */


/**
 * @class PropFilter
 * This class is used for accessing all properties of property list with a
 * given identifier.
 */


/**
 * @fn void PropFilter::look(void);
 * Look for the next property matching the identifier.
 */


/**
 * @fn PropFilter::PropFilter(PropList& list, Identifier *_id);
 * Build an iterator on properties matching the given name.
 * @param list	Property list to traverse.
 * @param _id	Looked identifier.
 */


/**
 * @fn PropFilter::PropFilter(PropList& list, Identifier& _id);
 * Build an iterator on properties matching the given name.
 * @param list	Property list to traverse.
 * @param _id	Looked identifier.
 */


/**
 * @fn void PropFilter::next(void);
 * Go to the next property.
 */


/**
 * @fn bool PropFilter::ended(void) const;
 * Test if there is still a property to examine.
 * @return	True if traversal is ended.
 */


/**
 * @fn Property *PropFilter::get(void) const;
 * Get the current property.
 */



/**
 * @fn PropFilter::operator bool(void) const;
 * Test if the traversal may continue.
 * @return	True if there is still porperties to examine.
 */


/**
 * @fn PropFilter::operator Property *(void) const;
 * Automatic conversion of iterator to property.
 * @return	Current property.
 */


/**
 * @fn PropFilter& PropFilter::operator++(void);
 * Shortcut to next() method call.
 */


/**
 * @fn Property *PropFilter::operator->(void) const;
 * Shortcut for accessing a member of the current property.
 */


} // otawa

