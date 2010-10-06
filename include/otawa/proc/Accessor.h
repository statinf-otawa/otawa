/*
 *	$Id$
 *	AbstractAccessor and Accessor classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-10, IRIT UPS.
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
#ifndef OTAWA_PROC_ACCESSOR_H_
#define OTAWA_PROC_ACCESSOR_H_

#include <otawa/prop/AbstractIdentifier.h>

namespace otawa {

class WorkSpace;

class AbstractAccessor: public AbstractIdentifier {
public:
	inline AbstractAccessor(cstring name = ""): AbstractIdentifier(name) { }
};

template <class T>
class Accessor: public AbstractAccessor {
public:
	inline Accessor(cstring name = ""): AbstractAccessor(name) { }
	virtual T get(WorkSpace *ws) const = 0;
};

template <class T>
class FunAccessor: public Accessor<T> {
public:
	typedef T (*fun_t)(WorkSpace *ws);
	inline FunAccessor(fun_t _f, cstring name = ""): Accessor<T>(name), f(_f) { }
	virtual T get(WorkSpace *ws) const { return f(ws); }
private:
	fun_t f;
};

}	// otawa

#endif /* OTAWA_PROC_ACCESSOR_H_ */
