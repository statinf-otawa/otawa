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

class Alias;
class Expression;
class System;

// Var class
class Var {
public:
	typedef enum {
		NO_TYPE = 0,
		INT = 1,
		BIN = 2,
		FLOAT = 3
	} type_t;

	virtual ~Var(void);
	inline const string& name(void) const { return _name; }
	inline type_t type(void) const { return _type; }
	virtual void print(io::Output& out);
	virtual Alias *toAlias(void);
	virtual double eval(System *sys);

protected:
	inline Var(type_t type = INT): _type(type) { }
	inline Var(cstring name, type_t type = INT): _name(name), _type(type) { }
	inline Var(const string& name, type_t type = INT): _name(name), _type(type) { }

private:
	string _name;
	type_t _type;
};

// Alias class
class Alias: public Var {
public:
	Alias(Expression *expr);
	Alias(string name, Expression *expr);
	virtual ~Alias(void);
	virtual Alias *toAlias(void);
	virtual double eval(System *sys);
	inline const Expression *expression(void) const { return _expr; }
private:
	Expression *_expr;
};

// Output
inline io::Output& operator<<(io::Output& out, Var *var) { var->print(out); return out; }
inline io::Output& operator<<(io::Output& out, Var& var) { var.print(out); return out; }
io::Output& operator<<(io::Output& out, Var::type_t type);


} } // otawa::ilp

#endif	// OTAWA_ILP_VAR_H
