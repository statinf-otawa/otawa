/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/Loader.cpp -- implementation for Loader class.
 */

#include <otawa/prog/Loader.h>

namespace otawa {

/**
 * @class Loader
 * This interface is implemented by all objects that may build and provide
 * a process. Many kind of loader may exists from the simple binary
 * loader to the complex program builder.
 * 
 * The loader may put on the created processes zero or several of the following
 * features according the machine-level feature provided:
 * @li @ref FLOAT_MEMORY_ACCESS_FEATURE
 * @li @ref MEMORY_ACCESS_FEATURE
 * @li @ref REGISTER_USAGE_FEATURE
 * @li @ref STACK_USAGE_FEATURE
 */


/**
 * Build a loader plugin with the given description.
 * @param name				Name of the loader.
 * @param version			Version of the loader.
 * @param plugger_version	Version of the plugger (must be OTAWA_LOADER_VERSION).
 * @param aliases			Name aliases.
 */
Loader::Loader(
	CString name,
	Version version,
	Version plugger_version,
	const Plugin::aliases_t& aliases)
: Plugin(name, plugger_version, OTAWA_LOADER_NAME, aliases) {
	_plugin_version = version;
}


/**
 * @fn Loader::~Loader(void)
 * Virtual destructor for destruction customization.
 */

/**
 * @fn CString Loader::getName(void) const
 * Get the name of the loader.
 * @return	Name of the loader.
 */

/**
 * @fn Process *Loader::load(Manager *man, CString path, PropList& props)
 * Load the file matching the given path with the given properties. The exact
 * type of the file and of the properties depends upon the underlying loader.
 * @param man		Caller manager.
 * @param path		Path to the file to load.
 * @param props	Property for loading.
 * @return Process containing the loaded file.
 * @exception LoadException							Error during the load.
 * @exception UnsupportedPlatformException	Loader does not handle
 * this platform.
 */

/**
 * @fn Process *Loader::create(Manager *man, PropList& props)
 * Build a new empty process matching the given properties.
 * @param man		Caller manager.
 * @param props	Properties describing the process.
 * @return Created process.
 * @exception UnsupportedPlatformException	Loader does not handle
 */


/**
 * Check the loader used in the given workspace matches the given name
 * and version.
 * @param ws				Current workspace.
 * @param name				Name to check.
 * @param version			Version to check.
 * @throw otawa::Exception	If the loader does not match.
 */
void Loader::check(WorkSpace *ws, cstring name, const Version& version) {
	Loader *loader = ws->process()->loader();
	ASSERT(loader);

	// check name and aliases
	bool found_name = true;
	if(loader->name() != name) {
		found_name = false;
		const Plugin::aliases_t& aliases = loader->aliases();
		for(int i = 0; i < aliases.count(); i++)
			if(name == aliases[i]) {
				found_name = true;
				break;
			}

	// check version
	if(!found_name && !version.accepts(loader->pluginVersion()))
		throw otawa::Exception(_ << name << " loader is required!");
	}
}

/**
 * Name of the "Heptane" loader.
 */
//CString Loader::LOADER_NAME_Heptane = "heptane";

/**
 * Name of the "GLISS PowerPC" loader.
 */
//CString Loader::LOADER_NAME_Gliss_PowerPC = "gliss-powerpc";

/**
 * Name of the default PowerPC platform.
 */
//CString Loader::PLATFORM_NAME_PowerPC_Gliss = "powerpc/ppc601-elf/gliss-gliss";
	
} // otawa
