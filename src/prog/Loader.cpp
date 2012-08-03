/*
 *	Load and gliss::Info implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-12, IRIT UPS <casse@irit.fr>
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/prog/Loader.h>
#include <otawa/loader/gliss.h>

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

namespace gliss {

/**
 * @class Info
 * This class provides access to the stay-behind GLISS data structures and code
 * provided by the GLISS v2 utility used by some loaders. This allows to re-use
 * GLISS to embed ISA-specific processing inside some analysis (typically pipeline analysis).
 *
 * The existence of this object is tied to the gliss::INFO_FEATURE and linked to the process
 * with the identifier gliss::Info::ID.
 */


/**
 * @fn bool Info::check(t::uint32 checksum);
 *
 * As the validity of the data used in Gliss is strongly coupled
 * with the data of an analysis using this object, OTAWA provides
 * a way to generate a checksum of the file isa/id.h and to compare both
 * values stored in the loader and in the analysis with this function.
 * If the check fails, it is utterly risky to use this structure.
 *
 * @param	Checksum to check.
 * @return	True, both checksum are compatible, false else.
 */


/**
 * @fn void *Info::decode(Inst *inst);
 * Decode an instruction and return its descriptor.
 * The returned descriptor must be released with Info::free().
 * @param inst	Instruction to decode.
 * @return		Decoded instruction.
 */


/**
 * void Info::free(void *desc);
 * Free a GLISS instruction descriptor previously allocated by Info::decode().
 * @param desc	Instruction descriptor to release.
 */


/**
 * @fn void Info::decode<T>(Inst *inst, T *&desc);
 * A type-wise way to perform the decoding of an instruction.
 * The returned descriptor must be fried with Info::free().
 */


/**
 */
Info::~Info(void) {
}


/**
 * This feature that the current process has been built by a loader
 * based on the GLISS tool. This allows to have low-level access
 * to GLISS primitives.
 *
 * @par Properties
 * @li gliss::Info::ID
 */
Feature<NoProcessor> INFO_FEATURE("otawa::gliss::INFO_FEATURE");

/**
 * This identifier allows to get the gliss::Info object from
 * loader supporting it from a process generating it.
 *
 * @par Features
 * @li @ref INFO_FEATURE
 *
 * @par Hooks
 * @li @ref Process
 */
Identifier<Info *> INFO("otawa::gliss::INFO", 0);

}	// gliss

} // otawa
