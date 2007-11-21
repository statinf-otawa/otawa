/*
 *	$Id$
 *	Var class implementation
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
#include <otawa/ilp/Var.h>

namespace otawa { namespace ilp {

/**
 * @class Var
 * A variable is an identifier used for performing ILP computation.
 * A variable may named or not and may inserted as any property. Have just
 * a thought about releasing it.
 */



/**
 * @fn Var::Var(void);
 * Build an anonymous variable.
 */


/**
 * @fn Var::Var(cstring *name);
 * Build a variable with the given name.
 * @param name	Name of the variable.
 */


/**
 * @fn Var::Var(const string& name);
 * Build a variable with the given name.
 * @param name	Name of the variable.
 */


/**
 * @fn String& Var::name(void);
 * Get the name of the variable if any. Return an empty string if there is none.
 * @return	Variable name.
 */


/**
 */
Var::~Var(void) {
}


/**
 * Print the name of the variable.
 * @param out	Output to use.
 */
void Var::print(io::Output& out) {
	if(_name)
		out << _name;
	else
		out << '_' << io::hex((int)this);
}

} } // otawa::ilp
