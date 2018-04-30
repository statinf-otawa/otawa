/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/Simulator.h -- Simulator class interface.
 */
#ifndef OTAWA_SIM_SIMULATOR_H
#define OTAWA_SIM_SIMULATOR_H

#include <otawa/hard/Platform.h>
#include <elm/sys/Plugin.h>

#include "../prop.h"

namespace otawa {
	
// External class
class WorkSpace;

namespace sim {
class State;

// Definitions
#define OTAWA_SIMULATOR_HOOK 	sim_plugin
#define OTAWA_SIMULATOR_NAME	"sim_plugin"
#define OTAWA_SIMULATOR_VERSION Version(1, 0, 0)


// Simulator configuration
extern Identifier<bool> IS_FUNCTIONAL;
extern Identifier<bool> IS_STRUCTURAL;
extern Identifier<bool> USE_MEMORY;
extern Identifier<bool> USE_CONTROL;


// Simulator class
class Simulator: public elm::sys::Plugin {
public:
	Simulator(elm::CString name,
		const elm::Version& version,
		const elm::Version& plugger_version);
	virtual State *instantiate(WorkSpace *fw,
		const PropList& props = PropList::EMPTY) = 0;
};

// Exception
class Exception: otawa::Exception {
	elm::String header(const Simulator& sim, const elm::CString message);
public:
 	Exception(const Simulator& sim, const string& message);
};

} } // otawa::sim

#endif /* OTAWA_SIM_SIMULATOR_H */
