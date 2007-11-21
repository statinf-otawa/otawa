/*
 *	$Id$
 *	Var class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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
#ifndef OTAWA_ILP_VAR_H
#define OTAWA_ILP_VAR_H

#include <elm/io.h>

namespace otawa { namespace ilp {

using namespace elm;

// Var class
class Var {
public:
	virtual ~Var(void);
	inline elm::String& name(void) { return _name; }
	virtual void print(io::Output& out);

protected:
	inline Var(void) { }
	inline Var(cstring name): _name(name) { }
	inline Var(const string& name): _name(name) { }

private:
	string _name;
};

// Output
inline io::Output& operator<<(io::Output& out, Var *var) {
	out << "ilp::Var("; var->print(out); out << ')';
	return out;
}

inline io::Output& operator<<(io::Output& out, Var& var) {
	out << "ilp::Var("; var.print(out); out << ')';
	return out;	
}

} } // otawa::ilp

#endif	// OTAWA_ILP_VAR_H
