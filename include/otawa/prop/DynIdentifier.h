/*
 *	$Id$
 *	DynIdentifier class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
#ifndef OTAWA_PROC_DYNIDENTIFIER_H_
#define OTAWA_PROC_DYNIDENTIFIER_H_

#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/prop/Identifier.h>

namespace otawa {

// DynIdentifier class
template <class T>
class DynIdentifier {
public:
	inline DynIdentifier(cstring name): _name(name), _id(0) { }
	inline cstring name(void) const { return _name; }

	const T &operator()(const PropList &props) const { return id()(props); }
	const T &operator()(const PropList *props) const { return id()(props); }
	Ref<T, Identifier<T> > operator()(PropList &props) const { return id()(props); }
	Ref<T, Identifier<T> > operator()(PropList *props) const { return id()(props); }

private:
	inline Identifier<T>& id(void) const {
		if(!_id)
			_id = (Identifier<T> *)ProcessorPlugin::getIdentifier(_name);
		ASSERTP(_id, _ << "identifier " << _name << " cannot be found.");
		return *_id;
	}

	cstring _name;
	mutable Identifier<T> *_id;
};

}	// otawa

#endif /* OTAWA_PROC_DYNIDENTIFIER_H_ */
