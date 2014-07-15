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
#include <otawa/ilp/Expression.h>
#include <otawa/ilp/System.h>

namespace otawa { namespace ilp {

/**
 * @class Var
 * A variable is an identifier used for performing ILP computation.
 * A variable may named or not and may inserted as any property. Have just
 * a thought about releasing it.
 */



/**
 * @fn Var::Var(type_t type);
 * Build an anonymous variable.
 * @param type	Type of the variable (default @ref INT).
 */


/**
 * @fn Var::Var(cstring *name, type_t type);
 * Build a variable with the given name.
 * @param name	Name of the variable.
 * @param type	Type of the variable (default @ref INT).
 * @deprecated
 */


/**
 * @fn Var::Var(const string& name, type_t type);
 * Build a variable with the given name.
 * @param name	Name of the variable.
 * @param type	Type of the variable (default @ref INT).
 */


/**
 * @fn String& Var::name(void);
 * Get the name of the variable if any. Return an empty string if there is none.
 * @return	Variable name.
 */


/**
 * @fn type_t Var::type(void) const;
 * Get the type of the variable.
 * @return	Variable type.
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
		out << '_' << (void *)this;
}


/**
 * If the variable is an alias, return it.
 * @return	Alias object if it is alias, null else.
 */
Alias *Var::toAlias(void) {
	return 0;
}


/**
 * Evaluate the variable in the given system.
 * @param sys	Resolved system to look in.
 * @return		Value of the variable.
 */
double Var::eval(System *sys) {
	return sys->valueOf(this);
}


/**
 * @enum Var::type_t
 * Type of an ILP variable.
 */


/**
 * @var Var::type_t Var::NO_TYPE;
 * Null value for ILP variable type.
 */


/**
 * @var Var::type_t Var::INT;
 * Integer (default) type for ILP variable.
 */


/**
 * @var Var::type_t Var::BIN;
 * Binary type (0 or 1) for ILP variable.
 */


/**
 * @var Var::type_t Var::FLOAT;
 * Real type for ILP variable.
 */


/**
 */
io::Output operator<<(io::Output& out, Var::type_t type) {
	cstring msgs[] = {
		"null",
		"int",
		"bin",
		"float"
	};
	ASSERT(type >= 0 && type <= Var::FLOAT);
	out << msgs[type];
	return out;
}


/**
 * @class Alias
 * An alias is a specific variable that represents an expressions.
 * This allows to reduce the number of true variable and to replace any variable
 * by any possible expression.
 *
 * @ingroup ilp
 */


/**
 * Build an anonymous alias variable.
 * @param expr	Expression of the alias (this object is responsible for freeing it).
 */
Alias::Alias(Expression *expr): _expr(expr) {
}


/**
 * Build a named alias variable.
 * @param name	Name of the variable.
 * @param expr	Expression of the alias (this object is responsible for freeing it).
 */
Alias::Alias(string name, Expression *expr): Var(name), _expr(expr) {
}


/**
 */
Alias::~Alias(void) {
	delete _expr;
}


/**
 */
Alias *Alias::toAlias(void) {
	return this;
}


/**
 */
double Alias::eval(System *sys) {
	return _expr->eval(sys);
}


/**
 * @fn const Expression *expression(void) const;
 * Get the expression of the alias.
 * @return	Alias expression.
 */

} } // otawa::ilp
