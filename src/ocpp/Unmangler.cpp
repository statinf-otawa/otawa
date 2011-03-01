/*
 *	$Id$
 *	Unmangler class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <stdlib.h>
#include <elm/assert.h>
#include <otawa/cpp/Unmangler.h>

namespace otawa { namespace cpp {

// NOTE: supported by MinGW
namespace abi {
	extern "C" char *__cxa_demangle (
		const char *mangled_name,
		char *buf,
		size_t *n,
		int *status);
}

// IA64 unmangler (used by G++)
class IA64Unmangler: public Unmangler {
public:

	virtual string unmangle(const string& name) throw(UnmanglingException) {
		int status;
		char *res = abi::__cxa_demangle(name.toCString(), 0, 0, &status);
		switch(status) {
		case 0: {
				string r(res);
				free(res);
				return r;
			}
		case -1:
			throw UnmanglingException("no more memory");
		case -2:
			throw UnmanglingException("bad format");
		default:
			throw UnmanglingException(_ << "unknown error: " << status);
		}
	}
};


// unmangler switcher
class BaseUnmangler: public Unmangler {
public:
	virtual string unmangle(const string& name) throw(UnmanglingException) {
		if(name.startsWith("_Z"))
			return ia64.unmangle(name);
		else
			return name;
	}

private:
	IA64Unmangler ia64;
};
static BaseUnmangler __base;

/**
 * @class UnmanglingException
 * This exception is raised when an error is found during demangling.
 * Requires linking with libocpp.
 */


/**
 * @class Unmangler
 * Unmangler of C++ symbols. Requires linking with libocpp.
 */


/**
 * Base unmangler. According to the name to unmangle, try to find
 * the right unmangler.
 */
Unmangler& Unmangler::base = __base;


/**
 * @fn string Unmangler::unmangle(const string& name) throw(UnmanglingException);
 * Perform unmangling of the given name.
 * @param name					Name to unmangle.
 * @return						Return the unmangled string.
 * @throw UnmanglingException	Thrown if there is an error during unmangling.
 */

} } // otawa::cpp
