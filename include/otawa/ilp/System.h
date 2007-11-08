/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	ilp/System.h -- interface to the System interface.
 */
#ifndef OTAWA_ILP_SYSTEM_H
#define OTAWA_ILP_SYSTEM_H

#include <elm/io.h>
#include <elm/string.h>
#include <otawa/ilp/Constraint.h>

namespace otawa { namespace ilp {

using namespace elm;

// System class
class System {
public:
	virtual Constraint *newConstraint(Constraint::comparator_t comp,
		double constant = 0) = 0;
	virtual bool solve(void) = 0;
	virtual void addObjectFunction(double coef, Var *var = 0) = 0;
	virtual double valueOf(Var *var) = 0;
	virtual double value(void) = 0;
	virtual int countVars(void) = 0;
	virtual int countConstraints(void) = 0;
	
	virtual void exportLP(io::Output& out = elm::cout) = 0;
	virtual void dumpSystem(io::Output& out = elm::cout);
	virtual void dumpSolution(io::Output& out = elm::cout) = 0;
	virtual void dump(elm::io::OutStream& out = elm::io::stdout);
};
	
} }	// otawa::ilp

#endif // OTAWA_ILP_SYSTEM_H
