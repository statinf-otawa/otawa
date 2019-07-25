/*
 *	NamedObject interface
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
#ifndef OTAWA_SCRIPT_NAMEDOBJECT_H_
#define OTAWA_SCRIPT_NAMEDOBJECT_H_

#include <elm/avl/Map.h>
#include <elm/serial2/macros.h>

namespace otawa { namespace script {

using namespace elm;

class NamedObject {
private:
	SERIALIZABLE(NamedObject, DFIELD(name, string("")));
public:
	static void declare(NamedObject *object);
	static NamedObject *get(string name);

	inline NamedObject(void) { }
	NamedObject(string _name);
	virtual ~NamedObject(void);
	inline string getName(void) const { return name; }
	template <class T> bool implements(void) const
		{ return T::__type->asClass().baseOf(this->__actual_class()); }

private:
	static avl::Map<string, NamedObject *> objects;
	string name;
};

} }		// otawa::script

#endif /* OTAWA_SCRIPT_NAMEDOBJECT_H_ */
