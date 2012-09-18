/*
 *	NamedObject implementation
 *	Copyright (c) 2012, IRIT UPS <casse@irit.fr>
 *
 *	LBlockBuilder class interface
 *	This file is part of OTAWA
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
 *	along with Foobar; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/script/NamedObject.h>

namespace otawa { namespace script {

/**
 * @class NamedObject
 * Named are used by script to locate a data structure passed as argument
 * to a code processor. Their name allows to easily design them from the script.
 * Classic children classes of this one includes @ref hard::Memory, @ref hard::Processor, etc.
 * As serializable objects, named objects allows also to check dynamically their type.
 */


/**
 * Known named objects.
 */
genstruct::AVLMap<string, NamedObject *> NamedObject::objects;


/**
 * Declare a new named object.
 * @param object	Object to declare.
 */
void NamedObject::declare(NamedObject *object) {
	objects.put(object->getName(), object);
}

/**
 * Get an object by its name.
 * @param name	Named of the looked object.
 * @return		Found object or null.
 */
NamedObject *NamedObject::get(string name) {
	return objects.get(name, 0);
}


/**
 * @fn NamedObject::NamedObject(void);
 * Anonymous named object constructor.
 */


/**
 * Constructor with a name. The declare() function
 * is automatically called, if the name is not empty, to make it public.
 * @param name	Name of the object.
 */
NamedObject::NamedObject(string _name): name(_name) {
	if(name)
		declare(this);
}


/**
 */
NamedObject::~NamedObject(void) {
}


/**
 * @fn string NamedObject::getName(void) const;
 * Get the name of the object.
 * @return	Object name.
 */

} }	// otawa::script

SERIALIZE(otawa::script::NamedObject);
