/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ilp_Var.h -- implementation of the Var class.
 */

#include <otawa/ilp/Var.h>

namespace otawa { namespace ilp {

/**
 * @class Var
 * A variable is an identifier used for performing ILP computation.
 * A variable may named or not and may inserted as any property. Have just
 * a thought about releasing it.
 */



/**
 * @fn Var::Var(void);
 * Build an anonymous variable.
 */


/**
 * @fn Var::Var(const char *name);
 * Build a variable with the given name.
 * @param name	Name of the variable.
 */


/**
 * @fn Var::Var(String& name);
 * Build a variable with the given name.
 * @param name	Name of the variable.
 */


/**
 * @fn String& Var::name(void);
 * Get the name of the variable if any. Return an empty string if there is none.
 * @return	Variable name.
 */


} } // otawa::ilp
