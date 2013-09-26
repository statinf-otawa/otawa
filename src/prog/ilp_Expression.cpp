/*
 *	ilp::Expression class implementation
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
#include <otawa/ilp/Expression.h>
#include <otawa/ilp/Var.h>
#include <otawa/ilp/System.h>

namespace otawa { namespace ilp {


/**
 * @class Expression
 * An expression allows to represent a sum of terms and may be used to represent
 * the value of an aliased variable.
 *
 * @ingroup ilp
 */


/**
 * @fn Expression::Expression(void);
 * Build an empty expression.
 */


/**
 * @fn Expression::Expression(const Expression *expr);
 * Build an expression by cloning the given one.
 * @param expr	Expression to clone.
 */


/**
 * Add a term to the expression.
 * @param coef	Coefficient of the term.
 * @param var	Variable of the term.
 */
void Expression::add(coef_t coef, ilp::Var *var) {
	if(!var || !var->toAlias())
		terms.add(Term(var, coef));
	else
		add(var->toAlias()->expression(), coef);
}


/**
 * @fn void Expression::sub(coef_t coef, ilp::Var *var);
 * Subtract a term to the expression.
 * @param coef	Coefficient of the term.
 * @param var	Variable of the term.
 */


/**
 * Perform the addition to the current expression
 * of the given expression multiplied by the coefficient.
 * @param expr	Expression to add.
 * @param coef	Multiplication coefficient.
 */
void Expression::add(const Expression *expr, coef_t coef) {
	for(Expression::Iterator term(expr); term; term++)
		terms.add(Term((*term).fst, (*term).snd * coef));
}


/**
 * Perform the subtraction from the current expression
 * of the given expression multiplied by the coefficient.
 * @param expr	Expression to subtract.
 * @param coef	Multiplication coefficient.
 */
void Expression::sub(const Expression *expr, coef_t coef) {
	add(expr, -coef);
}


/**
 * Multiply all terms of the expression by the given coefficient.
 * @param coef	Coefficient to multiply with.
 */
void Expression::mul(coef_t coef) {
	for(Iterator term(this); term; term++)
		terms.set(term, Term((*term).fst, (*term).snd * coef));
}


/**
 * Divide all terms of the expression by the given coefficient.
 * @param coef	Coefficient to divide with.
 */
void Expression::div(coef_t coef) {
	for(Iterator term(this); term; term++)
		terms.set(term, Term((*term).fst, (*term).snd / coef));
}


/**
 * Evaluate the expression using the variables solution of the given ILP system.
 * @param sys	System to look for variable value?
 * @return		Value of the expression in the system.
 */
double Expression::eval(System *sys) {
	double r = 0;
	for(Iterator term(this); term; term++)
		r += sys->valueOf((*term).fst) * (*term).snd;
	return r;
}


} }		// otawa::ilp
