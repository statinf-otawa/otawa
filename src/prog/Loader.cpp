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
 */

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
 * This property identifier is used for passing a cache hierarchy configuration
 * to the loader (const CacheConfiguration *).
 */
Identifier Loader::ID_Caches("otawa.caches");


/**
 * Identifier of the property indicating the name (CString) of the platform to use.
 */	
id_t Loader::ID_PlatformName = Property::getID("otawa.PlatformName");

/**
 * Identifier of the property indicating a name (CString) of the loader to use..
 */	
id_t Loader::ID_LoaderName = Property::getID("otawa.LoaderName");

/**
 * Identifier of the property indicating a platform (Platform *) to use.
 */	
id_t Loader::ID_Platform = Property::getID("otawa.Platform");

/**
 * Identifier of the property indicating the loader to use.
 */	
id_t Loader::ID_Loader = Property::getID("otawa.Loader");

/**
 * Identifier of the property indicating the identifier (PlatformId) of the loader to use.
 */	
id_t Loader::ID_PlatformId = Property::getID("otawa.PlatformId");

/**
 * Argument count as passed to the program (int).
 */	
id_t Loader::ID_Argc = Property::getID("otawa.Argc");

/**
 * Argument values as passed to the program (char **).
 */	
id_t Loader::ID_Argv = Property::getID("otawa.Argv");

/**
 * Argument values as passed to the program (char **).
 */	
id_t Loader::ID_Envp = Property::getID("otawa.Envp");


/**
 * Name of the "Heptane" loader.
 */
CString Loader::LOADER_NAME_Heptane = "heptane";

/**
 * Name of the "GLISS PowerPC" loader.
 */
CString Loader::LOADER_NAME_Gliss_PowerPC = "gliss-powerpc";

/**
 * Name of the default PowerPC platform.
 */
CString Loader::PLATFORM_NAME_PowerPC_Gliss = "powerpc/ppc601-elf/gliss-gliss";
	
} // otawa
