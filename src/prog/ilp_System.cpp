/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ilp_System.h -- documentation of the system interface.
 */

#include <otawa/otawa.h>
#include <otawa/ilp/System.h>

namespace otawa { namespace ilp {

/**
 * @class System
 * An ILP system is a colletion of ILP constraint that may maximize or minimize
 * some object function.
 */


/**
 * @fn Constraint *Sytem::newConstraint(Constraint::comparator_t comp, double constant = 0);
 * Build a new constraint that may be initialized by the user. As the ILP system
 * manage the memory of the constraint, the constraint must never be deleted
 * by the user.
 * @param comp		Comparator used (one of ILP_xxx).
 * @param constant	Constant value.
 * @return			Built constraint.
 */


/**
 * @fn boolean System::solve(void);
 * Solve the ILP system.
 * @return	True if the resolution is succesful or false else (mainly due
 * 			to lack of constraint).
 */


/**
 * @fn void System::addObjectFunction(double coef, Var *var = 0);
 * Add a factor to the object function.
 * @param coef	Coefficient of the factor.
 * @param var	Variable of the factor.
 */


/**
 * @fn double System::valueOf(Var *var);
 * This method can only be called after the resolution of the system and returns
 * the value of a variable. It is an error to pass a variable not involved in
 * the system.
 * @param var	Variable whose value is looked for.
 * @return		Value of the variable.
 */


/**
 * @fn double System::value(void);
 * Return the value of the optimized object function.
 * @return	Object function optimum.
 */

/**
 * @fn Var *System::newVar(void);
 * Build a new artifact variable, that is, a variable usable in the ILP system
 * but that is not linked to any other item from the OTAWA program
 * representation. Its life ends with the destruction of the ILP system.
 * @return	Created ILP variable.
 */


/**
 * Dump the system to the given output. The dumping format depends upon the
 * actual used ILP engine. Usually, it is compatible with other tools of
 * the ILP engine.
 * @param out	Used output.
 * @deprecated Use dumpSystem() and dumpSolution() instead.
 */
void System::dump(elm::io::OutStream& out) {
	io::Output output(out);
	dumpSystem(output);
	dumpSolution(output);
}


/**
 * @fn int System::countVars(void);
 * Count the number of variables in the system.
 * @return	Variable count.
 */


/**
 * @fn int System::countConstraints(void);
 * Count the number of constraints in the system.
 * @return	Constraint count.
 */


/**
 * @fn void System::exportLP(io::Output& out);
 * Export the system to the given output using the LP format (lp_solve).
 * @param out	Stream to export to (default to cout).
 */


/**
 * Dump the system in text format (as a default, call exportLP).
 * @param out	Stream to dump to (default to cout).
 */
void System::dumpSystem(io::Output& out) {
	exportLP(out);
}


/**
 * @fn void System::dumpSolution(io::Output& out);
 * Dump the solution textually to the given output.
 * @param out	Stream to output to (default to cout).
 */

} // ilp

} // otawa
