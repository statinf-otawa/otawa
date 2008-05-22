/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ilp/Constraint.h -- interface to the Constraint interface.
 */
#ifndef OTAWA_ILP_CONSTRAINT_H
#define OTAWA_ILP_CONSTRAINT_H

#include <otawa/ilp/Var.h>
#include <elm/Iterator.h>
#include <elm/utility.h>

namespace otawa { namespace ilp {
	
// Constraint class
class Constraint {
public:
	typedef elm::Pair<ilp::Var*,double> Term;
	typedef enum comparator_t {
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

	virtual void add(double coef, Var *var = 0) = 0;
	virtual void sub(double coef, Var *var = 0) = 0;
	
	virtual elm::IteratorInst<Term> *terms(void) = 0;
	inline void addLeft(double coef, Var *var = 0);
	inline void addRight(double coef, Var *var = 0);
	
	class TermIterator: public elm::Iterator<Term> {
		public:
		
		inline TermIterator(elm::IteratorInst<Term> *_inst) : elm::Iterator<Term>(_inst) {

		}
		inline TermIterator(Constraint *_cons) : elm::Iterator<Term>(_cons->terms()) {

		}	

	};	
	
};


// Inlines
inline void Constraint::addLeft(double coef, Var *var) {
	add(coef, var);
}

inline void Constraint::addRight(double coef, Var *var) {
	add(-coef, var);
}

} } // otawa::ilp

#endif	// OTAWA_ILP_CONSTRAINT_H
