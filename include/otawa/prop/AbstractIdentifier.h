/*
 * OTAWA -- WCET computation framework
 * Copyright (C) 2007-08  IRIT - UPS <casse@irit.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.package ojawa;
 */
#ifndef OTAWA_PROP_ABSTRACT_IDENTIFIER_H
#define OTAWA_PROP_ABSTRACT_IDENTIFIER_H

#include <assert.h>
#include <elm/string.h>
#include <elm/io.h>
#include <elm/util/VarArg.h>
#include <otawa/base.h>
#include <otawa/prop/PropList.h>

namespace otawa {

// Identifier class
class AbstractIdentifier: public PropList {
	friend class elm::Initializer<AbstractIdentifier>;

public:
	static AbstractIdentifier *find(const string& name);

	AbstractIdentifier(void);
	AbstractIdentifier(elm::String name);
	virtual ~AbstractIdentifier(void) { }

	inline const elm::String name(void) const { return nam; }

	virtual void print(elm::io::Output& out) const;
	virtual void print(elm::io::Output& output, const Property& prop) const;
	inline void print(elm::io::Output& output, const Property *prop) const { print(output, *prop); }
	virtual const Type& type(void) const;
	virtual void scan(PropList& props, VarArg& args) const;

private:
	elm::String nam;
	void initialize(void);
};


// DuplicateIdentifierException class
class DuplicateIdentifierException: public otawa::Exception {
public:
	DuplicateIdentifierException(String& name);
};

inline elm::io::Output& operator<<(elm::io::Output& out, const AbstractIdentifier& id) {
	id.print(out);
	return out;
}

inline elm::io::Output& operator<<(elm::io::Output& out, const AbstractIdentifier *id) {
	ASSERT(id);
	return out << *id;
}

} //otawa

#endif	// OTAWA_PROP_ABSTRACT_IDENTIFIER_H
