/*
 *	ilp impl module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_ILP_IMPL_H_
#define OTAWA_ILP_IMPL_H_

#include "Var.h"
#include "Constraint.h"
#include "System.h"

namespace otawa { namespace ilp {

class VarImpl: public Var {
public:
	inline VarImpl(void) { }
	inline VarImpl(string name): Var(name) { }
};


class ConstraintImpl: public Constraint {
public:
	ConstraintImpl(comparator_t comparator, string label = "");
	virtual double coefficient(Var *var) const;
	virtual double constant(void) const;
	virtual comparator_t comparator(void) const;
	virtual const string& label(void) const;

	virtual void add(double coef, Var *var);
	virtual void sub(double coef, Var *var);

	virtual dyndata::AbstractIter<Term> *terms(void);

private:
	Expression expr;
	comparator_t comp;
	string lab;
};

} }	// otawa::ilp

#endif /* OTAWA_ILP_IMPL_H_ */
