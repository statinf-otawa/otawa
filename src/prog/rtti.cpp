/*
 *	rtti module
 *	Copyright (c) 2018, IRIT UPS <casse@irit.fr>
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

#include <elm/rtti/Class.h>
#include <elm/rtti/type_of.h>
#include <otawa/properties.h>

using namespace elm;

namespace otawa {
extern rtti::Class<PropList> prop_list_type;
}

namespace elm { namespace rtti {
template <> inline const Type& _type<otawa::PropList>::_(void) { return otawa::prop_list_type; }
} }

namespace otawa {

rtti::Class<PropList> prop_list_type(rtti::make("otawa::PropList")
	.construct<PropList>("construct"));

}	// otawa

