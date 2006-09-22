/*
 *	$Id$
 *	Copyright (c) 2005, IRIT-UPS.
 *
 *	src/prog/ilp_ILPlLugin.cpp -- ILPPlugin class implementation.
 */

#include <otawa/ilp/ILPPlugin.h>

namespace otawa { namespace ilp {

/**
 * @class ILPPlugin
 * This interface must be implemented by plugins providing ILP processors.
 */


/**
 * Build the plugin.
 * @param name				Plugin name.
 * @param plugger_version	Required plugger version.
 * @param					Allow having static plugins (@see elm::system::Plugin).
 */
ILPPlugin::ILPPlugin(
		elm::CString name,
		const elm::Version& version,
		const elm::Version& plugger_version)
: Plugin(name, plugger_version, OTAWA_ILP_NAME) {
		_plugin_version = version;
}


/**
 * @fn System *ILPPLugin::newSystem(void);
 * Build a new system ready for use.
 * @return	New ILP system.
 */

} } // otawa::ilp
