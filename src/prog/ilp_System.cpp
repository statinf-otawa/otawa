/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ilp_System.h -- documentation of the system interface.
 */

#include <otawa/otawa.h>
#include <otawa/ilp/System.h>
#include <config.h>
#ifdef HAVE_LP_SOLVE
#	include <otawa/lp_solve/System.h>
#endif


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
 * @fn void System::dump(elm::io::OutStream& out);
 * Dump the system to the given output. The dumping format depends upon the
 * actual used ILP engine. Usually, it is compatible with other tools of
 * the ILP engine.
 * @param out	Used output.
 */


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

} // ilp

/*
 *  !!IMPORT!!
 * This method should be contained in FrameWork.cpp but, for avoiding mandatory
 * linkage with ILP engine, it has been moved here.
 */

/**
 * Build an ILP system with the default ILP engine.
 * @param max	True for a maximized system, false for a minimized.
 * @return		ILP system ready to use, NULL fi there is no support for ILP.
 */
ilp::System *FrameWork::newILPSystem(bool max) {
#	ifdef HAVE_LP_SOLVE
		return new lp_solve::System(max);
#	else
		return 0;
#	endif
}

} // otawa
