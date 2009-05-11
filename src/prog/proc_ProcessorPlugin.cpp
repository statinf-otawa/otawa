/*
 *	$Id$
 *	ProcessorPlugin class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#include <elm/system/Plugger.h>
#include <elm/system/Path.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/proc/Registry.h>
#include "config.h"

namespace otawa {

// external property of a feature
extern Identifier<bool> IS_FEATURE;


// processor plugger
static elm::system::Plugger plugger(OTAWA_PROC_NAME, OTAWA_PROC_VERSION);
static bool initialized_paths = false;


// build a canonical name
static string makeCanonical(string name) {
	StringBuffer buf;
	int start = 0, end = name.indexOf("::");
	while(end > -1) {
		buf << name.substring(start, end - start) << '/';
		start = end + 2;
		end = name.indexOf("::", start);
	}
	buf << name.substring(start);
	return buf.toString();
}


/**
 * @class ProcessorPlugin
 * This class must implemented by plugins containing processor, features
 * and identifiers.
 */


/**
 * Build a new processor plugin.
 * @param name				Name of the plugin (may be full-qualified C++ name with "::"
 * 							or canonical name with "/").
 * @param version			Version of the plugin.
 * @param plugger_version	Version for the plugger interface (OTAWA_PROC_VERSION).
 */
ProcessorPlugin::ProcessorPlugin(
	cstring name,
	const elm::Version& version,
	const elm::Version& plugger_version)
:	elm::system::Plugin(makeCanonical(name), plugger_version, OTAWA_PROC_NAME) {
	_plugin_version = version;
}


/**
 * @fn elm::genstruct::Table<AbstractRegistration&> ProcessorPlugin::processors(void) const;
 * This method must return the table of all processor available in the plugin.
 * @return	Table of available processors.
 */


/**
 * Initialize the plugger.
 */
void ProcessorPlugin::init(void) {
	initialized_paths = true;
	plugger.resetPaths();
	system::Path path = system::Path::current().append(".otawa/proc");
	plugger.addPath(path);
	path = system::Path::home().append(".otawa/proc");
	plugger.addPath(path);
	path = Path(PROC_PATHS);
	plugger.addPath(path);
}

/**
 * Find a plugin by its name. The looked directories are, in order,
 * "$PWD/.otawa/proc", "$HOME/.otawa/proc" or "$OTAWA_INSTALL_DIRECTORY/lib/otawa/proc".
 * The full path name of the processor class must be passed in the name, something
 * composed of "comp1::comp2::...::compn". First, a module whose path is
 * by replacing in processor name "::" by "/" is looked. If not found, the last component
 * of  the path is removed and the obtained path is looked again for module. This process
 * continue until the module is found or the path becomes empty resulting a linkage failure.
 *
 * @param name	Full-qualified name of the processor.
 * @return		Built processor or null if the processor cannot be found.
 */
ProcessorPlugin *ProcessorPlugin::get(string name) {
	if(!initialized_paths)
		init();

	// get canonical name
	string cname = makeCanonical(name);

	// iterates on components
	while(true) {
		int pos = cname.lastIndexOf('/');
		ProcessorPlugin *plugin = (ProcessorPlugin *)plugger.plug(cname);
		if(plugin)
			return plugin;
		if(pos < 0)
			return 0;
		cname = cname.substring(0, pos);
	}
}


/**
 * Add a path to the list of looked paths.
 * @param path	Path to add.
 */
void ProcessorPlugin::addPath(const elm::system::Path& path) {
	plugger.addPath(path);
}


/**
 * Remove the given path from the looked path.
 * @param path	Path to remove.
 */
void ProcessorPlugin::removePath(const elm::system::Path& path) {
	plugger.removePath(path);
}


/**
 * Get a processor by its name, possibly loading required plugin.
 * @param name	Name of the looked processor.
 * @return		An instance of the processor or NULL if it cannot be found.
 */
Processor *ProcessorPlugin::getProcessor(cstring name) {
	const AbstractRegistration *reg = Registry::find(name);
	if(!reg && get(name))
		reg = Registry::find(name);
	if(!reg)
		return 0;
	else
		return reg->make();
}


/**
 * Find a feature by its name, possibly loading a plugin.
 * @param name	Name of the looked feature.
 * @return		Found feature or null.
 */
AbstractFeature *ProcessorPlugin::getFeature(cstring name) {
	AbstractIdentifier *id = getIdentifier(name);
	if(IS_FEATURE(id))
		return (AbstractFeature *)id;
	else
		return 0;
}


/**
 * Find an identifier by its name possibly loading a plugin
 * to get it.
 * @param name	Name of the identifier.
 *
 */
AbstractIdentifier *ProcessorPlugin::getIdentifier(cstring name) {
	AbstractIdentifier *id = AbstractIdentifier::find(name);
	if(!id && get(name))
		id = AbstractIdentifier::find(name);
	return id;
}

}	// otawa

