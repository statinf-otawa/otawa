/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	manager.cc -- manager classes implementation.
 */

#include <config.h>
#include <otawa/platform.h>
#include <otawa/manager.h>
#include <otawa/cfg.h>
#include <otawa/ilp/ILPPlugin.h>
#include <gel.h>

#define STRINGIZE(x)	#x

namespace otawa {


/**
 * @class LoadException
 * Exception thrown when a loader encounters an error during load.
 */

/**
 * Build a load exception with a simple message.
 * @param message	Exception message.
 */
LoadException::LoadException(const String& message): Exception(message) {
}

/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param args		Arguments of the format.
 */
LoadException::LoadException(const char *format, VarArg& args)
: Exception(format, args) {
}


/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
LoadException::LoadException(const char *format, ...) {
	VARARG_BEGIN(args, format)
		build(format, args);
	VARARG_END
}


/**
 * @class UnsupportedPlatformException
 * Exception thrown when a loader cannot handle a platform.
 */

/**
 * Build an unsupported platform exception with a simple message.
 * @param message	Exception message.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const String& message)
: Exception(message) {
}

/**
 * Build an unsupported platform exception with a formatted message.
 * @param format	Format of the message.
 * @param args		Arguments of the format.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const char *format,
VarArg& args): Exception(format, args) {
}


/**
 * Build an unsupported platform exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const char *format,
...) {
	VARARG_BEGIN(args, format)
		build(format, args);
	VARARG_END
}


/**
 * @class Manager
 * The manager class providesfacilities for storing, grouping and retrieving shared
 * resources like loaders and platforms.
 */

/**
 * Delete all used resources.
 */
Manager::~Manager(void) {
	for(int i = 0; i < frameworks.count(); i++)
		delete frameworks[i];
	for(int i = 0; i < platforms.count(); i++)
		delete platforms[i];
}


/**
 * Find the loader matching the given name.
 * @param name	Name of the loader to find.
 * @return				Found loader or null.
 * @note This function should also perform dynamic loading of load shared
 * library.
 */
Loader *Manager::findLoader(CString name) {
	return (Loader *)loader_plugger.plug(name);
}

/**
 * Find a platform matching the given name.
 * @param name	Name of the platform to find.
 * @return				Found platform or null.
 */
hard::Platform *Manager::findPlatform(CString name) {
	return findPlatform(hard::Platform::Identification(name));
}

/**
 * Find a platform matching the given platform identifier.
 * @param id	Identifier of the platform.
 * @result			Found platform or null.
 */
hard::Platform *Manager::findPlatform(const hard::Platform::Identification& id) {
	for(int i = 0; i < platforms.count(); i++)
		if(platforms[i]->accept(id))
			return platforms[i];
	return 0;
}

/**
 * Load a file with the given path and the given properties.
 * @param path		Path of the file to load.
 * @param props		Properties describing the load process. It may contains the
 * properties : @ref TASK_ENTRY, @ref PLATFORM, @ref LOADER, @ref PLATFORM_NAME,
 * @ref LOADER_NAME, @ref ARGC, @ref ARGV, @ref ENVP, @ref SIMULATOR,
 * @ref CACHE_CONFIG, @ref PIPELINE_DEPTH.
 * @return The loaded framework or 0.
 */
FrameWork *Manager::load(CString path, PropList& props) {
	Process *proc = 0;
	
	// Simple identified loader
	Loader *loader = LOADER(props);
	if(!loader) {
		CString name = LOADER_NAME(props);
		if(name)
			loader = findLoader(name);
	}
	
	// Try with gel
	if(!loader) {
		gel_file_t *file = gel_open((char *)&path, 0, 0);
		if(!file)
			throw LoadException(gel_strerror());
		gel_file_info_t infos;
		gel_file_infos(file, &infos);
		StringBuffer buf;
		buf << "elf_" << infos.machine;
		gel_close(file);
		String name = buf.toString();
		loader = findLoader(name.toCString());
	}
	
	// Return result
	if(!loader)
		throw LoadException("no loader for \"%s\".", &path);
	else
		return new FrameWork(loader->load(this, path, props));
}

/**
 * Manager builder. Install the PPC GLISS loader.
 */
Manager::Manager(void):
	ilp_plugger("ilp_plugin", Version(1, 0, 0), ILP_PATHS),
	loader_plugger(STRINGIZE(OTAWA_LOADER_HOOK), OTAWA_LOADER_VERSION, LOADER_PATHS)
{
}


/**
 * Make an ILP system from the given plugin or from a named plugin.
 * @param name	Name of the plugin to use or an empty string for the
 * default plugin.
 * @return		A new ILP system ready to use or null (plugin not available).
 */
ilp::System *Manager::newILPSystem(String name) {
	ilp::ILPPlugin *plugin;
	
	// Select the first available plugin
	if(!name) {
		system::Plugger::Iterator plug(ilp_plugger);
		if(plug.ended())
			return 0;
		plugin = (ilp::ILPPlugin *)plug.plug();
	}
	
	// Find a plugin
	else {
		plugin = (ilp::ILPPlugin *)ilp_plugger.plug(name.toCString());
		if(!plugin) {
			cerr << "ERROR: " << ilp_plugger.lastErrorMessage() << "\n";
			return 0;
		}
	}

	// Return a system
	return plugin->newSystem();
}

}	// otawa
