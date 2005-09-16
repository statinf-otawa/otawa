/*
 *	$Id$
 *	Copyright (c) 2005, IRIT-UPS.
 *
 *	otawa/plugins/ILPlLugin.h -- ILPPlugin class interface.
 */
#ifndef OTAWA_PLUGINS_ILP_PLUGIN_H
#define OTAWA_PLUGINS_ILP_PLUGIN_H

#include <elm/system/Plugin.h>

namespace otawa { namespace ilp {
	
// External class
class System;

// ILPPlugin class
class ILPPlugin: public elm::system::Plugin {
public:
	ILPPlugin(elm::String name, const elm::Version& plugger_version,
		elm::String hook = "");
	virtual System *newSystem(void) = 0;
};

} } // otawa::ilp

#endif //OTAWA_PLUGINS_ILP_PLUGIN_H
