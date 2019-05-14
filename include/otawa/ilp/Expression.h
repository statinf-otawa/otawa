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

#include <elm/data/List.h>

namespace otawa { namespace ilp {

using namespace elm;

class System;
class Var;

typedef double coef_t;

class Term {
public:
	inline Term(coef_t c, Var *v = 0): fst(v), snd(c) { }
	inline Term(Var *v, coef_t c = 1.): fst(v), snd(c) { }
	Var *fst;
	coef_t snd;
};

class Expression {
public:
	inline Expression(void) { }
	inline Expression(const Expression *expr): terms(expr->terms) { }

	void add(coef_t coef, ilp::Var *var = 0);
	inline void sub(coef_t coef, ilp::Var *var = 0) { terms.add(Term(var, -coef)); }
	inline void add(const Term& t) { add(t.snd, t.fst); }
	inline void sub(const Term& t) { sub(t.snd, t.fst); }
	void add(const Expression *expr, coef_t coef = 1);
	void sub(const Expression *expr, coef_t coef = 1);
	void mul(coef_t coef);
	void div(coef_t coef);
	void add(const Expression& e);
	void sub(const Expression& e);

	double eval(System *sys);

	class Iter: public List<Term>::Iter {
	public:
		inline Iter(void) { }
		inline Iter(const Expression *expr): List<Term>::Iter(expr->terms) { }
	};

	inline Iter begin(void) const { return Iter(this); }
	inline Iter end(void) const { return Iter(); }
	inline Iter operator*(void) const { return begin(); }

	inline void reset(void) { terms.clear(); }

private:
	List<Term> terms;
};

io::Output& operator<<(io::Output& out, const Expression& e);

} }		// otawa::ilp

#endif /* OTAWA_ILP_EXPRESSION_H_ */
