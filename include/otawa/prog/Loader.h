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
#define OTAWA_LOADER_VERSION	"1.2.0"
#define OTAWA_LOADER_HOOK		loader_plugin
#define OTAWA_LOADER_NAME		"loader_plugin"
#define OTAWA_LOADER_ID(name, version, date)	ELM_PLUGIN_ID(OTAWA_LOADER_NAME, name " V" version " (" date ") [" OTAWA_LOADER_VERSION "]")

// Loader class
class Loader: public elm::system::Plugin {
	friend class Manager;
protected:
	virtual ~Loader(void) { };
public:
	Loader(CString name, Version version, Version plugger_version,
	       const elm::system::Plugin::aliases_t & aliases
	       = elm::system::Plugin::aliases_t::EMPTY);

	virtual CString getName(void) const = 0;
	virtual Process *load(Manager *man, CString path, const PropList& props) = 0;
	virtual Process *create(Manager *man, const PropList& props) = 0;

	void check(WorkSpace *ws, cstring name, const Version& version);

};

} // otawa

#endif	// OTAWA_PROG_LOADER_H
