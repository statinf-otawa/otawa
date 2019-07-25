/*
 *	$Id$
 *	Identifier information
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
#ifndef OTAWA_PROP_INFO_H_
#define OTAWA_PROP_INFO_H_

#include <otawa/prop/Identifier.h>

namespace otawa {

// property maker
template <class T>
inline Property *make(const Identifier<T>& id, const T& v) {
	return new GenericProperty<T>(id, v);
}

// string adaptaters
inline Property *make(const Identifier<cstring>& id, const char *v)
	{ return make(id, cstring(v)); }
inline Property *make(const Identifier<string>& id, const char *v)
	{ return make(id, string(v)); }


// identifiers
extern Identifier<cstring> IDENTIFIER_ID;
extern Identifier<cstring> IDENTIFIER_LABEL;
extern Identifier<cstring> IDENTIFIER_DESC;
inline Property *idLabel(cstring label) { return make(IDENTIFIER_LABEL, label); }
inline Property *idDesc(cstring desc) { return make(IDENTIFIER_DESC, desc); }

}	// otawa

#endif /* OTAWA_PROP_INFO_H_ */
