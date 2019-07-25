/*
 *	$Id$
 *	ilp::Constraint class interface
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
#ifndef OTAWA_ILP_CONSTRAINT_H
#define OTAWA_ILP_CONSTRAINT_H

#include <elm/dyndata/Collection.h>
#include <elm/utility.h>
#include <otawa/ilp/Var.h>
#include "Expression.h"

namespace otawa { namespace ilp {

// Constraint class
class Constraint {
public:
	typedef otawa::ilp::Term Term;
	typedef enum comparator_t {
		UNDEF = -3,		// interface 1.2.0
		LT = -2,
		LE = -1,
		EQ = 0,
		GE = 1,
		GT = 2
	} comparator_t;

	virtual ~Constraint(void) { };

	virtual double coefficient(Var *var = 0) const = 0;
	virtual double constant(void) const = 0;
	virtual comparator_t comparator(void) const = 0;
	virtual const string& label(void) const = 0;

	virtual void add(double coef, Var *var = 0) = 0;
	virtual void sub(double coef, Var *var = 0) = 0;

	virtual dyndata::AbstractIter<Term> *terms(void) = 0;
	inline void addLeft(double coef, Var *var = 0)  { add(coef, var); }
	inline void addRight(double coef, Var *var = 0) { add(-coef, var); }

	// interface 1.2.0
	virtual void setComparator(comparator_t comp) = 0;
	virtual void setLabel(const string& label) = 0;
	inline void add(const Term& t) { add(t.snd, t.fst); }
	inline void sub(const Term& t) { sub(t.snd, t.fst); }
	void add(const Expression& e);
	void sub(const Expression& e);

	class TermIterator: public dyndata::Iter<Term> {
		public:
			inline TermIterator(dyndata::AbstractIter<Term> *_inst): dyndata::Iter<Term>(_inst) { }
			inline TermIterator(Constraint *_cons) : dyndata::Iter<Term>(_cons->terms()) {
		}

	};

	// interface 1.2.1
	virtual void reset(void) = 0;

};

io::Output& operator<<(io::Output& out, Constraint::comparator_t comp);
io::Output& operator<<(io::Output& out, const Term& t);

} } // otawa::ilp

#endif	// OTAWA_ILP_CONSTRAINT_H
