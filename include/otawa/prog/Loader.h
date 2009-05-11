/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/Loader.h -- interface for Loader class.
 */
#ifndef OTAWA_PROG_LOADER_H
#define OTAWA_PROG_LOADER_H

#include <elm/system/Plugin.h>
#include <otawa/base.h>
#include <otawa/properties.h>
#include <otawa/prog/Process.h>
#include <otawa/hard/Platform.h>

namespace otawa {

namespace sim {
	class Simulator;
}


// Useful defines
#define OTAWA_LOADER_VERSION	Version(1, 0, 0)
#define OTAWA_LOADER_HOOK		loader_plugin
#define OTAWA_LOADER_NAME		"loader_plugin"


// Loader class
class Loader: public elm::system::Plugin {
	friend class Manager;
protected:
	virtual ~Loader(void) { };
public:
	Loader(CString name, Version version, Version plugger_version,
		const system::Plugin::aliases_t & aliases
		= system::Plugin::aliases_t::EMPTY);

	// Method
	virtual CString getName(void) const = 0;
	virtual Process *load(Manager *man, CString path, const PropList& props) = 0;
	virtual Process *create(Manager *man, const PropList& props) = 0;

	// Default platform and loader
	static Loader& LOADER_Gliss_PowerPC;
	static Loader& LOADER_Heptane_PowerPC;
	static CString LOADER_NAME_Gliss_PowerPC;
	static CString LOADER_NAME_Heptane;
	static CString PLATFORM_NAME_PowerPC_Gliss;
};

} // otawa

#endif	// OTAWA_PROG_LOADER_H
