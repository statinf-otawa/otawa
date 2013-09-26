/*
 *	ilp::Expression class interface
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
#ifndef OTAWA_ILP_EXPRESSION_H_
#define OTAWA_ILP_EXPRESSION_H_

#include <elm/genstruct/SLList.h>

namespace otawa { namespace ilp {

using namespace elm;

class System;
class Var;

typedef double coef_t;
typedef elm::Pair<ilp::Var*, coef_t> Term;

class Expression {
public:
	inline Expression(void) { }
	inline Expression(const Expression *expr): terms(expr->terms) { }

	void add(coef_t coef, ilp::Var *var = 0);
	inline void sub(coef_t coef, ilp::Var *var = 0) { terms.add(Term(var, -coef)); }
	void add(const Expression *expr, coef_t coef = 1);
	void sub(const Expression *expr, coef_t coef = 1);
	void mul(coef_t coef);
	void div(coef_t coef);

	double eval(System *sys);

	class Iterator: public genstruct::SLList<Term>::Iterator {
	public:
		inline Iterator(const Expression *expr): genstruct::SLList<Term>::Iterator(expr->terms) { }
		inline Iterator(const Iterator& iter): genstruct::SLList<Term>::Iterator(iter) { }
		inline Iterator& operator=(const Iterator& iter) { genstruct::SLList<Term>::Iterator::operator=(iter); return *this; }
	};

private:
	genstruct::SLList<Term> terms;
};

} }		// otawa::ilp

#endif /* OTAWA_ILP_EXPRESSION_H_ */
