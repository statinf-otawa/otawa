/*
 *	$Id$
 *	Copyright (c) 2005, Institut de Recherche en Informatique de Toulouse.
 *
 *	otawa/lp_solve/Constraint.h -- lp_solve::Constraint class interface.
 */
#ifndef OTAWA_LP_SOLVE_CONSTRAINT_H
#define OTAWA_LP_SOLVE_CONSTRAINT_H

#include <elm/io.h>
#include <otawa/ilp.h>
#include <elm/assert.h>

namespace otawa { namespace lp_solve {

// Extern classes
class System;
class Var;

// Constraint class
class Constraint: public ilp::Constraint {
	friend class System;
public:
	inline Constraint(System *sys, comparator_t comp, double constant,
		Constraint *next);
	inline Constraint(System *system, const string& label, comparator_t comp,
		double constant, Constraint *next)
		: sys(system), nxt(next), facts(0), _label(label) { ASSERT(sys); }
	~Constraint(void);

	inline double constant(void) const;
	virtual const string& label(void) const { return _label; }
	inline Constraint *next(void) const;
	void fillRow(double *row);
	void resetRow(double *row);
	void dump(elm::io::Output& out);

	// ilp::Constraint overload	
	virtual double coefficient(ilp::Var *var = 0) const;
	virtual comparator_t comparator(void) const;
	virtual void add(double coef, ilp::Var *var = 0);
	virtual void sub(double coef, ilp::Var *var = 0);
	virtual elm::datastruct::IteratorInst<elm::Pair<otawa::ilp::Var*, double> >* terms() { ASSERTP("Not implemented with lpsolve 4", false); return NULL; }		
	
private:
	// Factor class
	class Factor {
		Factor *nxt;
		Var *var;
		double coef;
	public:
		inline Factor(double coefficient, Var *variable, Factor *next);
		inline Var *variable(void) const;
		inline double coefficient(void) const;
		inline Factor *next(void) const;
		inline void add(double value);
		inline Factor& operator+=(double value);
		inline Factor& operator-=(double value);
	};
	
	// Attributes
	System *sys;
	Constraint *nxt;
	Factor *facts;
	double cst;
	comparator_t cmp;
	string _label;
};

// Constraint::SolveInst inlines
inline Constraint::Factor::Factor(double coefficient, Var *variable,
Factor *next): nxt(next), var(variable), coef(coefficient) {
}

inline Var *Constraint::Factor::variable(void) const {
	return var;
}

inline double Constraint::Factor::coefficient(void) const {
	return coef;
}

inline void Constraint::Factor::add(double value) {
	coef += value;
}

inline Constraint::Factor& Constraint::Factor::operator+=(double value) {
	add(value);
	return *this;
}

inline Constraint::Factor& Constraint::Factor::operator-=(double value) {
	add(-value);
	return *this;
}

// Constraint inlines
inline Constraint::Constraint(System *system, ilp::Constraint::comparator_t comp,
double constant, Constraint *next): sys(system), nxt(next), facts(0),
cst(constant),  cmp(comp) {
	ASSERT(sys);
}

inline double Constraint::constant(void) const {
	return cst;
}

inline Constraint::Factor *Constraint::Factor::next(void) const {
	return nxt;
}

inline Constraint *Constraint::next(void) const {
	return nxt;
}

} }	// otawa::lp_solve

#endif	// OTAWA_LP_SOLVE_CONSTRAINT_H

