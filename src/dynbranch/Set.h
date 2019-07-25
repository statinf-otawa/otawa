/*
 *	Set class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
#ifndef OTAWA_DYNBRANCH_SET_H
#define OTAWA_DYNBRANCH_SET_H

#include <DBVector.h>

using namespace elm;

namespace otawa { namespace dynbranch {

class MyGC;

template <typename T>
class XSet : public Vector<T> {
public:

	XSet<T>(void) { ASSERT(0); }
	XSet<T>(const XSet<T>& set) : Vector<T>(set) { ASSERT(0);}

	XSet<T>(const XSet<T>& set, MyGC* gc) : Vector<T>(gc, set) { }
	XSet<T>(MyGC* gc) : Vector<T>(gc, 0) { }

/*
	void insert(const T& val) {
		if(!Vector<T>::contains(val))
			Vector<T>::push(val);
	}
*/
};

}}

#endif	// OTAWA_DYNBRANCH_SET_H
