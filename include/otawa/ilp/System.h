/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ilp/System.h -- interface to the System interface.
 */
#ifndef OTAWA_ILP_SYSTEM_H
#define OTAWA_ILP_SYSTEM_H

#include <otawa/ilp/Constraint.h>

namespace otawa { namespace ilp {

// System class
class System {
public:
	virtual Constraint *newConstraint(ilp_constraint_t comp, double constant = 0) = 0;
	virtual boolean solve(void) = 0;
	virtual void addObjectFunction(double coef, Var *var = 0) = 0;
	virtual double valueOf(Var *var) = 0;
	virtual double value(void) = 0;
};
	
} }	// otawa::ilp

#endif // OTAWA_ILP_SYSTEM_H
