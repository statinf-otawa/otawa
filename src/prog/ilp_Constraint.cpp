/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ilp_Constraint.h -- documentation of the Constraint interface.
 */

#include <otawa/ilp/Constraint.h>

namespace otawa { namespace ilp {

/**
 * @class Constraint
 * This class is used to represent constraints in an ILP system with the
 * following form:
 * @code
 * 	c1*x1 + c2*x2 + ... + cn*xn (=|<|>|<=|>=) k
 * @endcode
 * @li c1, c2, ..., cn are the coefficients of terms.<br>
 * @li x1, x2, ..., xn are the variables of terms.<br>
 * @li c1*x1, c2*x2, ..., cn*xn are called the terms.<br>
 * @li k is the constant part.
 *
 * @ingroup ilp
 */

/**
 * @fn double Constraint::coefficient(Var *var = 0) const;
 * Get the coefficient for the given variable.
 * @param var	Variable to get the coefficent of. Null variable allows getting
 * 				the constant part.
 * @return		Variable coefficient.
 */

/**
 * @fn Constraint::comparator_t Constraint::comparator(void) const;
 * Get the constraint comparator.
 * @return	Constraint comparator.
 */


/**
 * @fn void Constraint::add(double coef, Var *var = 0);
 * Add a term to the constraint to the left part of constraint. If var is null,
 * sum the coefficient to the right part of the constraint.
 * @param coef	Coefficient of the factor.
 * @param var	Variable of the factor. When null, modify the constant.
 */


/**
 * @fn void Constraint::sub(double coef, Var *var = 0);
 * Substact a factor from the constraint.
 * @param coef	Coefficient of the factor.
 * @param var	Variable of the factor.
 */


/**
 * @fn void Constraint::addLeft(double coef, Var *var = 0);
 * Add a factor on the left side.
 * @param coef	Coefficient of the factor.
 * @param var	Variable of the factor.
 */


/**
 * @fn void Constraint::addRight(double coef, Var *var = 0);
 * Add a factor on the right side.
 * @param coef	Coefficient of the factor.
 * @param var	Variable of the factor.
 */


/**
 * void Constraint::setComparator(comparator_t comp);
 * Change the comparator of the constraint.
 * @param comp	Comparator to set.
 * @since ILP interface 1.2.0
 */


/**
 * void Constraint::setLabel(const string& label);
 * Change the label of the constraint.
 * @param label		Label to set.
 * @since ILP interface 1.2.0
 */


/**
 * @fn void Constraint::reset(void);
 * Reset the constraint, i.e. remove all left and right members.
 * @snce ILP interface 1.2.1.
 */


/**
 * Add an expression to the left part of the constraint.
 * @param e		Added expression.
 */
void Constraint::add(const Expression& e) {
	for(Expression::Iter i(&e); i(); i++)
		add(*i);
}


/**
 * Subtract an expression from the left part of the constraint.
 * @param e		Subtracted expression.
 */
void Constraint::sub(const Expression& e) {
	for(Expression::Iter i(&e); i(); i++)
		sub(*i);
}


/**
 */
io::Output& operator<<(io::Output& out, const Term& t) {
	if(!t.snd)
		out << '0';
	else if(!t.fst)
		out << t.snd;
	else if(t.fst->name())
		out << t.snd << ' ' << t.fst->name();
	else
		out << t.snd << " x" << (void *)t.fst;
	return out;
}


/**
 */
io::Output& operator<<(io::Output& out, Constraint::comparator_t comp) {
	static cstring strs[] = {
		"<",	//	LT = -2,
		"<=",	// LE = -1,
		"=",	// EQ = 0,
		">=",	// GE = 1,
		">"		// GT = 2
	};
	ASSERT(comp >= Constraint::LT && comp < Constraint::GT);
	out << strs[comp + 2];
	return out;
}



} } // otawa::ilp
