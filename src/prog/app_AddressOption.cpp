/*
 *	$Id$
 *	AddressOption class implementation
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

#include <otawa/app/AddressOption.h>
#include <elm/io/BlockInStream.h>

namespace otawa {

using namespace elm;

/**
 * @class AddressOption
 * An option allowing to read an address, currently, only an unsigned decimal, hexadecimal
 * and so one value.
 */


/**
 */
AddressOption::AddressOption(option::Manager &manager, char short_name, cstring description, Address def)
: option::AbstractValueOption(Make(manager).cmd(_ << '-' << short_name).description(description)), addr(def)  {
}


/**
 */
AddressOption::AddressOption(option::Manager &manager, cstring long_name, cstring description, Address def)
: option::AbstractValueOption(Make(manager).cmd(long_name).description(description)), addr(def) {
}


/**
 */
AddressOption::AddressOption(option::Manager &manager, char short_name, cstring long_name, cstring description, Address def)
: option::AbstractValueOption(Make(manager).cmd(_ << '-' << short_name).cmd(long_name).description(description)), addr(def) {
}


/**
 * @fn Address AddressOption::address(void) const;
 * Get the address contained in the option.
 * @return	Address.
 */


/**
 */
option::usage_t AddressOption::usage(void) {
	return option::arg_required;
}


/**
 */
cstring AddressOption::argDescription(void) {
	return "ADDRESS";
}


/**
 */
void AddressOption::process(string arg) {
	try {
		t::uint32 a;
		arg >> a;
		addr = a;
	}
	catch(elm::Exception& exn) {
		throw option::OptionException("bad address format");
	}
}

} // otawa
