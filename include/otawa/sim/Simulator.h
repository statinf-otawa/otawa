/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/Simulator.h -- Simulator class interface.
 */
#ifndef OTAWA_SIM_SIMULATOR_H
#define OTAWA_SIM_SIMULATOR_H

#include <otawa/hard/Platform.h>
#include <elm/system/Plugin.h>
#include <otawa/properties.h>

namespace otawa {
	
// External class
class FrameWork;

namespace sim {
class State;

// Definitions
#define OTAWA_SIMULATOR_HOOK 	sim_plugin
#define OTAWA_SIMULATOR_NAME	"sim_plugin"
#define OTAWA_SIMULATOR_VERSION Version(1, 0, 0)


// Simulator configuration
extern GenericIdentifier<bool> IS_FUNCTIONAL;
extern GenericIdentifier<bool> IS_STRUCTURAL;
extern GenericIdentifier<bool> USE_MEMORY;
extern GenericIdentifier<bool> USE_CONTROL;


// Simulator class
class Simulator: public elm::system::Plugin {
public:
	Simulator(elm::CString name, const elm::Version& version,
		const elm::CString description = "",
		const elm::CString license = "");
	virtual State *instantiate(FrameWork *fw,
		const PropList& props = PropList::EMPTY) = 0;
};

// Exception
class Exception: otawa::Exception {
	elm::String header(const Simulator& sim, const elm::CString message);
public:
 	Exception(const Simulator& sim, elm::String& message);
 	Exception(const Simulator& sim, elm::CString format, elm::VarArg &args);
 	Exception(const Simulator& sim, elm::CString format,...);
};

} } // otawa::sim

#endif /* OTAWA_SIM_SIMULATOR_H */
