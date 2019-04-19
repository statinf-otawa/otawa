/*
 *	$Id$
 *	Unmangler class interface
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
#ifndef OTAWA_CPP_UNMANGLER_H
#define OTAWA_CPP_UNMANGLER_H

#include <elm/string.h>
#if defined(__WIN32) || defined(__WIN64)
#include <elm/util/MessageException.h>
#endif

namespace otawa { namespace cpp {

using namespace elm;

class UnmanglingException: public MessageException {
public:
	UnmanglingException(string message): MessageException(message) { }
};

class Unmangler {
public:
	static Unmangler& base;
	virtual string unmangle(const string& name) = 0;
};

} } // otawa::cpp

#endif	// OTAWA_CPP_UNMANGLER_H

