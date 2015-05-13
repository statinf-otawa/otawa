/*
 *	ilp::AbstractSystem class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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

#include <otawa/ilp/AbstractSystem.h>

namespace elm { namespace datastruct {

template <class T, class I>
class IteratorMaker: public datastruct::IteratorInst<T> {
public:
	inline IteratorMaker(const I& iter): i(iter) { }
	virtual ~IteratorMaker(void) { }
	virtual bool ended(void) const { return i.ended(); }
	virtual T item(void) const { return i.item(); }
	virtual void next(void) { i.next(); }
private:
	I i;
};

} }		// elm::datastruct

namespace otawa { namespace ilp {

/**
 * @class AbstractConstraint
 * Constraint generated by AbstractSystem. Provided publicly to allow
 * extension of AbstractSystem.
 *
 * @ingroup ilp
 */


/**
 * Build an abstract constraint.
 * @param label		Associated label.
 * @param comp		Constraint comparator.
 * @param cst		Numeric constant put to the right of the constraint.
 */
AbstractConstraint::AbstractConstraint(string label,  comparator_t comp, double cst)
: _label(label), _comp(comp), _cst(cst), _idx(0), _sys(0) {
}


/**
 */
AbstractConstraint::~AbstractConstraint(void) {
	ASSERT(_sys);
	_sys->remove(this);
}


/**
 */
double AbstractConstraint::coefficient(Var *var) const {
	if(!var)
		return constant();
	else {
		for(Expression::Iterator i(&_expr); i; i++)
			if((*i).fst == var)
				return (*i).snd;
			return 0;
	}
}


/**
 */
double AbstractConstraint::constant(void) const {
	return _cst;
}


/**
 */
Constraint::comparator_t AbstractConstraint::comparator(void) const {
	return _comp;
}


/**
 */
const string& AbstractConstraint::label(void) const {
	return _label;
}


/**
 */
void AbstractConstraint::add(double coef, Var *var) {
	if(!var)
		_cst -= coef;
	else
		_expr.add(coef, var);
}


/**
 */
void AbstractConstraint::sub(double coef, Var *var) {
	if(!var)
		_cst += coef;
	else
		_expr.sub(coef, var);
}


/**
 */
elm::datastruct::IteratorInst<Term> *AbstractConstraint::terms(void) {
	return new datastruct::IteratorMaker<Term, Expression::Iterator>(Expression::Iterator(&_expr));
}


/**
 * @class AbstractSystem
 * This class provides a convenient way to handle ILP systems in OTAWA.
 * The ILP system (constraints, variables) are stored in OTAWA the solve() method
 * is called. This method is not provided by AbstractSystem but the implementation
 * is left to a child class for customization: the child class can now visit the system
 * and built the equivalent one in a particular solver.
 *
 * @ingroup ilp
 */


/**
 */
AbstractSystem::~AbstractSystem(void) {
	cleaning = true;

	// free constraints
	for(genstruct::FragTable<AbstractConstraint *>::Iterator i(conss); i; i++)
		if(*i)
			delete *i;

	// free variables
	for(genstruct::FragTable<AbstractVar *>::Iterator i(vars); i; i++)
		if(*i)
			delete *i;

	cleaning = false;
}


/**
 */
Constraint *AbstractSystem::newConstraint(Constraint::comparator_t comp, double constant) {
	return newConstraint("", comp, constant);
}


/**
 */
Constraint *AbstractSystem::newConstraint(const string& label, Constraint::comparator_t comp, double constant) {
	AbstractConstraint *cons = new AbstractConstraint(label, comp, constant);
	cons->_sys = this;
	cons->_idx = conss.length();
	conss.add(cons);
	return cons;
}


/**
 */
void AbstractSystem::addObjectFunction(double coef, Var *var) {
	obj.add(coef, var);
}


/**
 */
Var *AbstractSystem::newVar(const string& name) {
	AbstractVar *var = new AbstractVar(name, Var::INT);
	var->_sys = this;
	var->_idx = vars.length();
	vars.add(var);
	return var;
}


/**
 */
int AbstractSystem::countVars(void) {
	return vars.count();
}


/**
 */
int AbstractSystem::countConstraints(void) {
	return conss.count();
}

/**
 */
elm::datastruct::IteratorInst<Constraint*> *AbstractSystem::constraints(void) {
	return new datastruct::IteratorMaker<Constraint *, ConstIter>(ConstIter(this));
}


/**
 */
elm::datastruct::IteratorInst<Constraint::Term> *AbstractSystem::objTerms(void) {
	return new datastruct::IteratorMaker<Term, Expression::Iterator>(Expression::Iterator(&obj));
}


/**
 */
void AbstractSystem::exportLP(io::Output& out) {
	dump(CPLEX, out.stream());
}


/**
 */
void AbstractSystem::dumpSolution(io::Output& out) {
	for(genstruct::FragTable<AbstractVar *>::Iterator var(vars); var; var++)
		if(*var)
			out << var->name() << " = " << valueOf(*var) << io::endl;
}


/**
 * Get index (in the system storage table) of the variable.
 * This method is useful to associate AbstractSystem variables
 * with the underlying ILP system.
 * @param var	Variable to get index for.
 * @return		Variable index.
 */
int AbstractSystem::index(ilp::Var *var) {
	return static_cast<AbstractVar *>(var)->_idx;
}


/**
 * Remove a constraint from the system.
 * @param cons	Removed constraint.
 */
void AbstractSystem::remove(AbstractConstraint *cons) {
	if(cleaning)
		return;
	int i = cons->_idx;
	AbstractConstraint *tcons = conss[conss.count() - 1];
	conss[i] = tcons;
	tcons->_idx = i;
	conss.shrink(conss.count() - 1);
}


/**
 * Remove a variable from the system.
 * @param var	Variable to remove.
 */
void AbstractSystem::remove(AbstractVar *var) {
	if(cleaning)
		return;
	int i = var->_idx;
	AbstractVar *tvar = vars[vars.count() - 1];
	vars[i] = tvar;
	tvar->_idx = i;
	vars.shrink(vars.count() - 1);
}


/**
 * @class AbstractVar
 * Variable of AbstractSystem.
 */


/**
 */
AbstractSystem::AbstractVar::AbstractVar(const string& name, type_t type)
: Var(name, type), _idx(0), _sys(0) { }


/**
 */
AbstractSystem::AbstractVar::~AbstractVar(void) {
	ASSERT(_sys);
	_sys->remove(this);
}

} }		// otawa::ilp

