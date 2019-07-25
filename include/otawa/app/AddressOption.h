/*
 *	$Id$
 *	AddressOption class interface
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
#ifndef OTAWA_APP_ADDRESSOPTION_H_
#define OTAWA_APP_ADDRESSOPTION_H_

#include <otawa/base.h>
#include <elm/option/ValueOption.h>

namespace otawa {

using namespace elm;

// AddressOption class
class AddressOption: public option::AbstractValueOption {
public:
	AddressOption(option::Manager &manager, char short_name, cstring description, Address def = Address::null);
	AddressOption(option::Manager &manager, cstring long_name, cstring description, Address def = Address::null);
	AddressOption(option::Manager &manager, char short_name, cstring long_name, cstring description, Address def = Address::null);
	AddressOption(const Make& make, Address def = Address::null): option::AbstractValueOption(make), addr(def) { }

	inline Address address(void) const { return addr; }
	inline Address operator*(void) const { return addr; }

	// option::Option overload
	virtual option::usage_t usage(void);
	virtual cstring argDescription(void);
	virtual void process(string arg);

private:
	Address addr;
};

} // otawa

#endif /* OTAWA_APP_ADDRESSOPTION_H_ */
