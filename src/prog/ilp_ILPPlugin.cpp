/*
 *	$Id$
 *	Copyright (c) 2005, IRIT-UPS.
 *
 *	src/prog/ilp_ILPlLugin.cpp -- ILPPlugin class implementation.
 */

#include <otawa/ilp/ILPPlugin.h>

namespace otawa { namespace ilp {

/**
 * @defgroup ilp ILP -- Integer Linear Programming
 *
 * ILP (Integer Linear Programming) is the OTAWA class set dedicated to solving ILP optimization problems.
 * For example, @ref IPET module uses it to compute WCET. ILP problem try to optimize (maximize or minimize)
 * a linear expression under a set of constraints. Constrains are made of linear expressions separated
 * by a comparator (one of =, <, <=, >, >=).
 *
 * @par ILP System
 *
 *  Basically, an ILP problem is represented by an @ref ilp::System object. This object allows
 *  to define the optimization direction (maximize or minimize), the objective expression
 *  and to create @ref Constraint.
 *
 * Both objective expression and @ref ilp::Constraint are made of sums of @ref ilp::Term.
 * A term is the multiplication of a variable @Var by a floating-point coefficient.
 *
 * As the set basic ILP classis (@ref Constraint, @ref Constraint and @ref Term) are not
 * very developer-friendly, we advice to use the expr programming model described below.
 *
 * @par expr Interface
 *
 * The expr interface allows writing C++ code that mimics mathematical way to write
 * linear expression in ILP and to improve readability of code generating constraints.
 * For example, the constraint "3 x + 5 y <= 10" will be written:
 * @code
 * 	#include <otawa/ilp/expr.h>
 *
 * 	model m(...);
 * 	var x(m), y(m);
 * 	m() + 3 * x + 5 * y <= 10;
 * @endcode
 * The @ref model construction accepts as argument a pointer of @ref ilp::System.
 *
 * If a sum of variables is needed at the left of the constraint, use the following form:
 * @code
 * 	cons s = m();
 * 	for(i = 0; i < N; i++)
 * 		s += v[i];
 * 	s <= 10;
 * @endcode
 *
 * If a sum of variables is needed at the right, use the following form:
 * @code
 * 	cons c = m() + 10 == 0.;
 * 	for(i = 0; i < N; i++)
 * 		c += v[i];
 * @endcode
 *
 * To get an ILP system, you can use the @ref Manager::newILPSystem() method for example.
 *
 * In fact, a constraint is built in two phases. In the first phase, the constraint object itself
 * is built in the system and stored in an @ref ilp::cons object after "m()". Addition of
 * following terms will be accumulated at the left of the created constraint. As soon as the inequality
 * operator is applied, the ilp::cons switches its state and the following additions of terms will be
 * accumulated in the right part of the constraint. This techniques
 * allows to handle efficiently the build of the constraint.
 *
 * A side-effect of this techniques is that literal accumulation of terms on the right is completely
 * inefficient. An object of type @ref ilp:Expression that stores a list of terms is built
 * and may duplicate itself according to the evaluation order of the right-side terms:
 * @code
 * 	m() + 3 <= 4 * x + 5 * y;
 * @endcode
 *
 * A better form would be:
 * @code
 * 	(m() + 3 <= 4 * x) + 5 * y;
 * @endcode
 *
 * As readability has been lost, simply reversing the constraint may prevent this issue:
 * @code
 * 	m() + 4 * x + 5 * y >= 3;
 * @endcode
 *
 * @section par ILP Plugin
 *
 * OTAWA does not provide its own ILP solver but proposes plugin connecting with well-known off-the-shelf
 * solvers like lp_solve or CPlex. The ILP plugins are usual plugins of OTAWA but must implements
 * the class ilp::ILPPlugin.
 */


/**
 * @class ILPPlugin
 * This interface must be implemented by plugins providing ILP processors.
 */


/**
 * Build the plugin.
 * @param name				Plugin name.
 * @param plugger_version	Required plugger version.
 * @param					Allow having static plugins (@see elm::system::Plugin).
 * @deprecated	Use ILPPlugin(make&) instead.
 */
ILPPlugin::ILPPlugin(
		elm::CString name,
		const elm::Version& version,
		const elm::Version& plugger_version)
: Plugin(name, plugger_version, OTAWA_ILP_NAME) {
		_plugin_version = version;
}


/**
 * Build an ILP plugin using a maker.
 * @param maker	Maker describing the plugin.
 */
ILPPlugin::ILPPlugin(make& maker): Plugin(maker.hook(OTAWA_ILP_NAME)) {
}


/**
 * @fn System *ILPPLugin::newSystem(bool max);
 * Build a new system ready for use.
 * @param max	True to maximize the system, false to minimize (default to true).
 * @return	New ILP system.
 */


/**
 * @class var
 * Encapsulation for ilp::Var pointers for @ref {ilp} expr user-fiendly interface.
 * A simple way to convert an ilp::Var pointer to a var is the function @ref x().
 * @ingroup ilp
 */


/**
 * @class cons
 * Encapsulation for ilp::Constraint pointers for @ref {ilp} expr user-fiendly interface.
 * @ingroup ilp
 */


/**
 * @class model
 * Encapsulation for ilp::System pointers for @ref {ilp} expr user-fiendly interface.
 * Notice the function call operators that allows to create a new constraint without,
 * operator(), or with a label, operator(const string&).
 * @ingroup ilp
 */


/**
 * @fn var x(ilp::Var *v);
 * Convert an ilp::Var * to a @ref var. Useful to work-around C++ conversion limitations
 * when the variable is stored in a property.
 * @param v		Variable to convert.
 * @return		Converted variable.
 * @ingroup ilp
 */
} } // otawa::ilp
