/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/lp_prog/stub.cpp -- stub for ILP engine management.
 */

#include <otawa/manager.h>
#include <otawa/prog/FrameWork.h>
#include <otawa/ilp/ILPPlugin.h>
#include <config.h>
#ifndef HAVE_PLUGIN
#	ifdef HAVE_LP_SOLVE
#		include <otawa/lp_solve/System.h>
#	endif
#endif

namespace otawa {
	
/**
 * Make an ILP system from the given plugin or from a named plugin.
 * @param name	Name of the plugin to use or an empty string for the
 * default plugin.
 * @return		A new ILP system ready to use or null (plugin not available).
 */
ilp::System *Manager::newILPSystem(String name) {
#	ifdef HAVE_PLUGIN
		ilp::ILPPlugin *plugin;
	
		// Select the first available plugin
		if(!name) {
			system::Plugger::Iterator plug(ilp_plugger);
			if(plug.ended())
				return 0;
			plugin = (ilp::ILPPlugin *)plug.plug();
			//cout << "Selected plugin : " << plugin->name() << "\n";
		}
	
		// Find a plugin
		else {
			plugin = (ilp::ILPPlugin *)ilp_plugger.plug(name);
			if(!plugin) {
				cerr << "ERROR: " << ilp_plugger.lastErrorMessage() << "\n";
				return 0;
			}
		}

		// Return a system
		return plugin->newSystem();
#	else
#		ifdef HAVE_LP_SOLVE
			if(!name || name == "lp_solve")
				return new lp_solve::System();
#		endif
		return 0;
#	endif
}


/**
 * Build an ILP system with the default ILP engine.
 * @param max	True for a maximized system, false for a minimized.
 * @return		ILP system ready to use, NULL fi there is no support for ILP.
 */
ilp::System *FrameWork::newILPSystem(bool max) {
#	ifdef HAVE_PLUGIN
		return manager()->newILPSystem();
#	else
#		ifdef HAVE_LP_SOLVE
			return new lp_solve::System();
#		else
			return 0;	
#		endif
#	endif
}

}
