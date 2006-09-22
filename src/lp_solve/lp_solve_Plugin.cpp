/*
 *	$Id$
 *	Copyright (c) 2005, IRIT-UPS.
 *
 *	src/lp_solve/LPSolvePlugin.cpp -- LPSolvePlugin class definition.
 */

#include <otawa/lp_solve/System.h>
#include <otawa/ilp/ILPPlugin.h>

using namespace elm;
using namespace otawa::ilp;

namespace otawa { namespace lp_solve {

// LPSolvePlugin class
class Plugin: public ILPPlugin {
public:
	Plugin(void);
	
	// ILPPlugin overload
	virtual System *newSystem(void);
};


/**
 * Build the plugin.
 */
Plugin::Plugin(void)
: ILPPlugin("lp_solve", Version(1, 0, 0), OTAWA_ILP_VERSION) {
}


/**
 */
System *Plugin::newSystem(void) {
	return new System();
}


} } // otawa::lp_solve

/**
 * Define the actual plugin.
 */
otawa::lp_solve::Plugin lp_solve_plugin;
otawa::lp_solve::Plugin& OTAWA_ILP_HOOK = lp_solve_plugin;

