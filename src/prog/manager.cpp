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
LoadException::LoadException(const char *format, va_list args): Exception(format, args) {
}


/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
LoadException::LoadException(const char *format, ...) {
	va_list args;
	va_start(args, format);
	build(format, args);
	va_end(args);
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
UnsupportedPlatformException::UnsupportedPlatformException(const char *format, va_list args)
: Exception(format, args) {
}


/**
 * Build an unsupported platform exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const char *format, ...) {
	va_list args;
	va_start(args, format);
	build(format, args);
	va_end(args);
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
	for(int i = 0; i < loaders.count(); i++)
		delete loaders[i];
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
	for(int i = 0; i < loaders.count(); i++)
		if(loaders[i]->getName() == name)
			return loaders[i];
	return 0;
}

/**
 * Find a platform matching the given name.
 * @param name	Name of the platform to find.
 * @return				Found platform or null.
 */
Platform *Manager::findPlatform(CString name) {
	return findPlatform(PlatformId(name));
}

/**
 * Find a platform matching the given platform identifier.
 * @param id	Identifier of the platform.
 * @result			Found platform or null.
 */
Platform *Manager::findPlatform(const PlatformId& id) {
	for(int i = 0; i < platforms.count(); i++)
		if(platforms[i]->accept(id))
			return platforms[i];
	return 0;
}

/**
 * Load a file with the given path and the given properties.
 * @param path		Path of the file to load.
 * @param props	Properties describing the load process. It must contains
 * a loder property like ID_Loader or ID_LoaderName.
 * @return The loaded framework or 0.
 */
FrameWork *Manager::load(CString path, PropList& props) {
	
	// Get the loader
	Loader *loader = props.get<Loader *>(Loader::ID_Loader, 0);
	if(!loader) {
		CString name = props.get<CString>(Loader::ID_LoaderName, "");
		if(!name)
			return 0;
		loader = findLoader(name);
		if(!loader)
			return 0;
	}
	
	// Attempt to load the file
	Process *proc = loader->load(this, path, props);
	if(!proc)
		return 0;
	else
		return new FrameWork(proc);
}

/**
 * Manager builder. Install the PPC GLISS loader.
 */
Manager::Manager(void)
: ilp_plugger("ilp_plugin", Version(1, 0, 0), ILP_PATHS) {
	//loaders.add(&gliss::loader);
	//cout << "ILP_PATHS = " << ILP_PATHS << "\n";
}

}
