/*
 *	ilp impl module implementation
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
#include <otawa/ilp/impl.h>

namespace otawa { namespace ilp {

/**
 * @class VarImpl
 * Very trivial implementation of the @ref Var class.
 */


/**
 * @fn VarImpl::VarImpl(void);
 * Anonymous variable constructor.
 */


/**
 * @fn VarImpl::VarImpl(string name);
 * Named variable constructor.
 * @param name	Name of the variable.
 */


/**
 * @class ConstraintImpl
 * Provide a simple implementation of a constraint using expression.
 * The stored expression e states only that: e {=|<|>|<=|>=} 0.
 * It is relatively easy to push back the constant to the right of the constraint.
 */


/**
 * Build an empty constraint.
 * @param comparator	Comparator to use.
 * @param label			Label of the constraint.
 */
ConstraintImpl::ConstraintImpl(comparator_t comparator, string label): comp(comparator), lab(label) {

}


/**
 */
double ConstraintImpl::coefficient(Var *var) {
	int c = 0;
	for(Expression::Iterator term(&expr); term; term++)
		if((*term).fst == var)
			c += (*term).snd;
	return var ? c : -c;
}


/**
 */
double ConstraintImpl::constant(void) const {
	int c = 0;
	for(Expression::Iterator term(&expr); term; term++)
		if(!(*term).fst)
			c += (*term).snd;
	return -c;
}


/**
 */
Constraint::comparator_t ConstraintImpl::comparator(void) const {
	return comp;
}


/**
 */
const string& ConstraintImpl::label(void) const {
	return lab;
}


/**
 */
void ConstraintImpl::add(double coef, Var *var) {
	expr.add(coef, var);
}


/**
 */
void ConstraintImpl::sub(double coef, Var *var) {
	expr.add(-coef, var);
}


class ConstraintTermIterator: public elm::datastruct::IteratorInst<Term>  {
public:
	ConstraintTermIterator(Expression *e): iter(e) { step(); }
	virtual bool ended(void) const { return iter.ended(); }
	virtual Term item(void) const { return iter.item(); }
	virtual void next(void) { iter.next(); step(); }
	inline void step(void) { while(!ended() && !(*iter).fst) iter.next(); }
private:
	Expression::Iterator iter;
};


/**
 */
elm::datastruct::IteratorInst<Term> *ConstraintImpl::terms(void) {
	return new ConstraintTermIterator(&expr);
}

} }		// otawa::ilp
