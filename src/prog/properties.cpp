/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	properties.cc -- property management.
 */

#include <elm/io.h>
#include <otawa/properties.h>
#include <elm/genstruct/HashTable.h>
using namespace elm;

namespace otawa {

/**
 * @class Property
 * A property associates a data with an identifier. They are stored in ProgObject
 * and allow storage and retrieval of any data. For efficiency purpose, they use
 * integer identifier that are associated with string names.
 */

// Property names store
static genstruct::HashTable<String, id_t> ids;
static id_t top_id = 1;

/**
 * Allocate a new identifier matching the given name. Note that, if an identifier
 * exists with the same name, its code will be returned ensuring consistency
 * between name and code for identifiers.
 * @param name	Name of the identifier.
 * @return				Allocated identifier code.
 */
id_t Property::getID(CString name) {
	String sname(name);
	
	// Already defined
	Option<id_t> result = ids.get(sname);
	if(result)
		return *result;
	
	// Create it
	id_t id = top_id++;
	ids.put(sname, id);
	return id;
}

/**
 * @fn Property::~Property(void)
 * Virtual destructor for allowing destruction of any stored data.
 */
 
/**
 * @fn Property::Property(id_t _id)
 * Build a new property with the given identifier.
 * @param _id	Identifier of the property.
 */

/**
 * @fn Property::Property(CString name)
 * Build a new property with the given identifier name. The matching identifier
 * code is automatically retrieved.
 */

/**
 * @fn id_t Property::getID(void) const
 * Get the identifier code of the property.
 * @return	Identifier code.
 */

/**
 * @fn Property *Property::getNext()
 * Get the next property.
 * @return Next property.
 */


/**
 * @class GenericProperty
 * This generic class allows attaching any type of data to a property.
 */

/**
 * @fn GenericProperty::GenericProperty(id_t id, T _value)
 * Build a new generic property with the given value.
 * @param id		Identifier code of the property.
 * @param _value	Value of the property.
 */

/**
 * @fn GenericProperty::GenericProperty(CString name, T _value)
 * Build a new generic property by its identifier name. The matching identifier
 * code is automatically retrieved.
 * @param name	Identifier name of the property.
 * @param _value	Value of the property.
 */

/**
 * @fn T& GenericProperty::getValue(void) 
 * Get the value of the property.
 * @return	Value of the property.
 */


/**
 * @class PropList
 * This a list of properties. This may be inherited for binding properties to
 * other classes or used as-is for passing heterogeneous properties to a function
 * call.
 */

/**
 * Find a property by its identifier.
 * @param id	Identifier of the property to find.
 * @return			Found property or null.
 */
Property *PropList::getProp(id_t id) {
	for(Property *cur = head, *prev = 0; cur; prev = cur, cur = cur->next)
		if(cur->id == id) {
			if(prev) {
				prev->next = cur->next;
				cur->next = head;
				head = cur;
			}
			return cur;
		}
	return 0;
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
			delete cur;
			break;
		}
	
	// Link the new property
	prop->next = head;
	head = prop;
}


/**
 * @fn void PropList::setProp(id_t id);
 * Set an indicator property (without value).
 * @param id	Identifier of the property.
 */

/**
 * Remove a property matching the given identifier.
 * @param id	Identifier of the property to remove.
 */
void PropList::removeProp(id_t id) {
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
void PropList::clear(void) {
	for(Property *cur = head, *next; cur; cur = next) {
		next = cur->next;
		delete cur;
	}
	head = 0;
}

/**
 * @fn T PropList::get(id_t id, T def_value);
 * Get the value of a property.
 * @param id			Identifier of the property to get.
 * @param def_value	Default value returned if property does not exists.
 * @return					Value of the property.
 */

/**
 * @fn Option<T> PropList::get(id_t id)
 * Get the value of a property.
 * @param id	Identifier of the property to get the value from.
 * @return			None or the value of the property.
 */

/**
 * @fn T& PropList::use(id_t id)
 * Get the reference on the value of the given property. If not found,
 * cause an assertion failure.
 * @param id	Identifier of the property to get the value from.
 * @return			Reference on the property value.
 */


/**
 * @fn void PropList::set(id_t id, const T value)
 * Set the value of a property.
 * @param id		Identifier of the property.
 * @param value	Value of the property.
 */

};
