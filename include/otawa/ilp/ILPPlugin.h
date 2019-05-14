/*
 *	$Id$
 *	Copyright (c) 2005, IRIT-UPS.
 *
 *	otawa/plugins/ILPlLugin.h -- ILPPlugin class interface.
 */
#ifndef OTAWA_PLUGINS_ILP_PLUGIN_H
#define OTAWA_PLUGINS_ILP_PLUGIN_H

#include <elm/sys/Plugin.h>

namespace otawa { namespace ilp {

// External class
class System;

// Definitions
#define OTAWA_ILP_HOOK		ilp_plugin
#define OTAWA_ILP_NAME		"ilp_plugin"
#define OTAWA_ILP_VERSION	"1.3.0"
#define OTAWA_ILP_ID(name, version, date)	ELM_PLUGIN_ID(OTAWA_ILP_NAME, name " V" version " (" date ") [" OTAWA_ILP_VERSION "]")

// ILPPlugin class
class ILPPlugin: public elm::sys::Plugin {
public:
	ILPPlugin(make& maker);
	ILPPlugin(
		elm::CString name,
		const elm::Version& version,
		const elm::Version& plugger_version);
	virtual System *newSystem(bool max = true) = 0;
};

} } // otawa::ilp

#endif //OTAWA_PLUGINS_ILP_PLUGIN_H
