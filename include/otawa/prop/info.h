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

// identifiers
extern Identifier<cstring> IDENTIFIER_LABEL;
extern Identifier<cstring> IDENTIFIER_DESC;

// SetString class
class SetCString {
public:
	inline SetCString(PropList& props, Identifier<cstring>& id, cstring str)
		{ id(props) = str; }
};

// macros
#define __UNIQUE_AUX2(x, y)	x##y
#define __UNIQUE_AUX(x, y)	__UNIQUE_AUX2(x, y)
#define __UNIQUE(pref)		__UNIQUE_AUX(pref, __COUNTER__)
#define SET_LABEL(id, label) \
	static SetCString __UNIQUE(__label_)(id, IDENTIFIER_LABEL, label);
#define SET_DESC(id, label)	 \
	static SetCString __UNIQUE(__desc_)(id, IDENTIFIER_DESC, desc);

}	// otawa

#endif /* OTAWA_PROP_INFO_H_ */
