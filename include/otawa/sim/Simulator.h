/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <casse@irit.fr>.
 *
 *	otawa/sim/Simulator.h -- Simulator class interface.
 */
#ifndef OTAWA_SIM_SIMULATOR_H
#define OTAWA_SIM_SIMULATOR_H

#include <elm/system/Plugin.h>
#include <otawa/base.h>

namespace otawa { namespace sim {
	
// External class
class FrameWork;
class State;

// Simulator class
class Simulator: public elm::system::Plugin {
public:
	static const elm::CString PLUGGER_NAME;
	static const elm::Version PLUGGER_VERSION;
	Simulator(elm::String name, const elm::Version& version,
		const elm::CString description = "",
		const elm::CString license = "");
	virtual State *instantiate(FrameWork *fw) = 0;
};

// Exception
class Exception: otawa::Exception {
	elm::String header(const Simulator& sim, const elm::CString message);
public:
 	Exception(const Simulator& sim, elm::String message);
 	Exception(const Simulator& sim, elm::CString format, elm::VarArg &args);
 	Exception(const Simulator& sim, elm::CString format,...);
};

} } // otawa::sim

#endif /* OTAWA_SIM_SIMULATOR_H */
