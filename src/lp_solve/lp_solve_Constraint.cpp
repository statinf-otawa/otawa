/*
 *	$Id$
 *	Copyright (c) 2005, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/lp_solve/lp_solve_Constraint.cpp -- lp_solve::Constraint class definition.
 */

#include <assert.h>
#include <otawa/lp_solve/System.h>
#include <otawa/lp_solve/Constraint.h>

namespace otawa { namespace lp_solve {

/**
 * @class Constraint
 * Constraint implementation for the lp_solve library.
 * Only stores information about the constraint:
 * <ul>
 * <li>constant value,</li>
 * <li>comparator,</li>
 * <li>factor (variables and coefficient).</li>
 * </ul>
 */


/**
 * Free all ressources.
 */
Constraint::~Constraint(void) {
	Factor *fact = facts;
	while(fact) {
		Factor *next = fact->next();
		delete fact;
		fact = next;
	}
	sys->removeConstraint(this);
}


// Overload
double Constraint::coefficient(ilp::Var *var) const {
	assert(var);
	Var *lvar = sys->getVar(var);
	assert(lvar);
	for(Factor *fact = facts; fact; fact = fact->next())
		if(fact->variable() == lvar)
			return fact->coefficient();
	assert(false);
}

// Overload
Constraint::comparator_t Constraint::comparator(void) const {
	return cmp;
}

// Overload
/* !!NOTE!!
 * Collection of factors would be more fast if they was stored in a dynamic
 * vector. Not done now for debugging purpose but should be performed.
 * Another interesting optimization should be to remove factor whose coefficient
 * become null.
 */
void Constraint::add(double coef, ilp::Var *var) {
	if(var == 0)
		cst += coef;
	else {
		Var *lvar = sys->getVar(var);
		assert(lvar);
		for(Factor *fact = facts; fact; fact = fact->next())
			if(fact->variable() == lvar) {
				fact->add(coef);
				return;
			}
		facts = new Factor(coef, lvar, facts);
	}
}

/**
 * Fill a row of the lp_solve matrice with the current constraint.
 * @param row	Row to fill.
 */
void Constraint::fillRow(double *row) {
	assert(row);
	for(Factor *fact = facts; fact; fact = fact->next())
		row[fact->variable()->column()] = fact->coefficient();
}


/**
 * Reset the value used in the given row by the current constraint.
 * @param row	Row to reset.
 */
void Constraint::resetRow(double *row) {
	assert(row);
	for(Factor *fact = facts; fact; fact = fact->next())
		row[fact->variable()->column()] = 0;
}


// Overload
void Constraint::sub(double coef, ilp::Var *var) {
	add(-coef, var);
}

} }	// otawa::lp_solve

