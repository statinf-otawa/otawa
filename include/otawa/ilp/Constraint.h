/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ilp/Constraint.h -- interface to the Constraint interface.
 */
#ifndef OTAWA_ILP_CONSTRAINT_H
#define OTAWA_ILP_CONSTRAINT_H

#include <otawa/ilp/Var.h>

namespace otawa { namespace ilp {
	
// Inequality type
typedef enum ilp_constraint_t {
	ILP_EQ,
	ILP_GT,
	ILP_GE,
	ILP_LT,
	ILP_LE
} ilp_constraint_t;

// Constraint class
class Constraint {
public:
	virtual double coef(Var *var = 0) const = 0;
	virtual ilp_constraint_t comp(void) const = 0;

	virtual void add(double coef, Var *var = 0) = 0;
	virtual void sub(double coef, Var *var = 0) = 0;
	
	inline void addLeft(double coef, Var *var = 0);
	inline void addRight(double coef, Var *var = 0);
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
