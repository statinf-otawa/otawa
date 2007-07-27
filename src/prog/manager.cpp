/*
 *	$Id$
 *	Manager class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
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

#include <config.h>
#include <otawa/platform.h>
#include <otawa/manager.h>
#include <otawa/cfg.h>
#include <otawa/ilp/ILPPlugin.h>
#include <gel/gel.h>
#include <elm/xom.h>
#include <otawa/sim/Simulator.h>

using namespace elm;

namespace otawa {

// Private
static String buildPaths(cstring kind, string paths) {
	StringBuffer buf;
	buf << "./.otawa/" << kind << ":"
		<< elm::system::Path::home() << "/.otawa/" << kind << ":"
		<< paths;
	return buf.toString();
}


/**
 * Namespace of otawa elements in an OTAWA configuration file.
 */
const CString Manager::OTAWA_NS = "http://www.irit.fr/recherches/ARCHI/MARCH";


/**
 * Name of the top element in an OTAWA configuration file.
 */
const CString Manager::OTAWA_NAME = "otawa";


/**
 * Name of the part describing a processor configuration in an OTAWA configuration
 * file.
 */
const CString Manager::PROCESSOR_NAME = "processor";


/**
 * Name of the XML element containing the cache configuration.
 */
const CString Manager::CACHE_CONFIG_NAME = "cache-config";


/**
 * @class LoadException
 * Exception thrown when a loader encounters an error during load.
 */


/**
 * Build a load exception with a formatted message.
 * @param message	Message.
 */
LoadException::LoadException(const String& message)
: MessageException(message) {
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
 * @note This function should also perform dynamic loading of shared
 * library.
 */
Loader *Manager::findLoader(CString name) {
	return (Loader *)loader_plugger.plug(name);
}


/**
 * Find the simulator matching the given name.
 * @param name	Name of the simulator to load.
 */
sim::Simulator *Manager::findSimulator(elm::CString name) {
	return (sim::Simulator *)sim_plugger.plug(name);
}


/**
 * Load a file with the given path and the given properties.
 * @param path		Path of the file to load.
 * @param props		Configuration properties.
 * @return 			The loaded workspace or null.
 * 
 * The configuration properties may be :
 * @li @ref ARGC,
 * @li @ref ARGV,
 * @li @ref CACHE_CONFIG,
 * @li @ref CACHE_CONFIG_ELEMENT,
 * @li @ref CACHE_CONFIG_PATH,
 * @li @ref CONFIG_ELEMENT (experimental),
 * @li @ref CONFIG_PATH (experimental),
 * @li @ref ENVP,
 * @li @ref ILP_PLUGIN_NAME,
 * @li @ref LOADER,
 * @li @ref LOADER_NAME,
 * @li @ref NO_STACK,
 * @li @ref NO_SYSTEM,
 * @li @ref PIPELINE_DEPTH,
 * @li @ref PLATFORM,
 * @li @ref PLATFORM_NAME,
 * @li @ref PROCESSOR,
 * @li @ref PROCESSOR_ELEMENT,
 * @li @ref PROCESSOR_PATH,
 * @li @ref SIMULATOR,
 * @li @ref SIMULATOR_NAME,
 * @li @ref TASK_ENTRY.
 * 
 * @par
 * 
 * This call try to link a plugin matching the Instruction Set Architecture
 * of the loaded binary file. This plugin is looked in the following directories:
 * @li $HOME/.otawa/loader
 * @li $PWD/.otawa/loader
 * @li <installation directory>/lib/otawa/loader
 * 
 * The two first cases let the user to provide their own plugin for, as an
 * example, to develop a new loader plugin.
 */
WorkSpace *Manager::load(const elm::system::Path&  path, const PropList& props) {
	
	// Just load binary ?
	if(path.extension() != "xml")
		return loadBin(path, props);
	
	// Load the XML file
	loadXML(path, props);
	return load(CONFIG_ELEMENT(props), props);
}


/**
 * Load a binary file.
 * @param path	Path to the binary file.
 * @param props	Configuration properties.
 * @return		Built framework.
 * @throws	LoadException	Error during load.
 */
WorkSpace *Manager::loadBin(
	const elm::system::Path& path,
	const PropList& props)
{
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
		if(!file) {
			throw LoadException(gel_strerror());
                }
		gel_file_info_t infos;
		gel_file_infos(file, &infos);
		StringBuffer buf;
		buf << "elf_" << infos.machine;
		gel_close(file);
		String name = buf.toString();
/*		cout << "Try to load " << name << " from " << LOADER_PATHS << io::endl; */
		if(Processor::VERBOSE(props)) {
			cerr << "INFO: looking for loader \"" << name << "\"\n";
			cerr << "INFO: available loaders\n";
			for(elm::system::Plugger::Iterator plugin(loader_plugger); plugin; plugin++)
				cerr << "INFO:\t- " << *plugin << io::endl;
		}
		loader = findLoader(name.toCString());
	}
	
	// No loader -> error
	if(!loader)
		throw LoadException(_ << "no loader for \"" << path << "\".");

	// Try to load the binary
	return new WorkSpace(loader->load(this, &path, props));
}


/**
 * Load an XML configuration file.
 * @param path	Path of the XML file.
 * @param props	Property to install the configuration in.
 * @return		Loaded workspace.
 * @throws	LoadException	Error during load.
 */ 
WorkSpace *Manager::loadXML(
	const elm::system::Path& path,
	const PropList& props)
{
	
	// Load the file
	xom::Builder builder;
	xom::Document *doc = builder.build(&path);
	if(!doc)
		throw LoadException(_ << "cannot load \"" << path << "\".");
	xom::Element *elem = doc->getRootElement();
	
	// Check the file
	if(elem->getLocalName() != "otawa"
	|| elem->getNamespaceURI() != OTAWA_NS)
		throw LoadException("not a valid OTAWA XML.");
	
	// Record the configuration
	if(Processor::VERBOSE(props))
		cout << "MANAGER: load configuration from \"" << path << "\".\n";
	PropList new_config;
	new_config.addProps(props);
	CONFIG_ELEMENT(new_config) = elem;
	return loadBin(path, new_config);
}


/**
 * Load the framework from an XML configuration.
 * @param elem	Top element of the configuration.
 * @param props	Configuration properties.
 * @return		Built workspace.
 * @throws	LoadException	Error during load.
 */
WorkSpace *Manager::load(xom::Element *elem, const PropList& props) {
	assert(elem);
	xom::Element *bin = elem->getFirstChildElement("binary");
	if(!bin)
		throw LoadException("no binary available.");
	elm::system::Path bin_path = (CString)bin->getAttributeValue("ref");
	if(!bin_path)
		throw LoadException("no binary available.");
	return loadBin(bin_path, props);
} 


/**
 * Load a computation configuration from the given properties.
 * @param props		Configuration properties.
 * @return			Load workspace.
 * @throws	LoadException	Error during load.
 */
WorkSpace *Manager::load(const PropList& props) {
	
	// Look for an XML element
	xom::Element *elem = CONFIG_ELEMENT(props);
	if(elem)
		return load(elem, props);
	
	// Look for a file name
	elm::system::Path path = CONFIG_PATH(props);
	if(path)
		return load(path, props);
	
	// Nothing to do
	throw LoadException("nothing to do.");
}


/**
 * Manager builder. Install the PPC GLISS loader.
 */
Manager::Manager(void):
	ilp_plugger("ilp_plugin", Version(1, 0, 0),
		buildPaths("ilp", ILP_PATHS)),
	loader_plugger(OTAWA_LOADER_NAME, OTAWA_LOADER_VERSION,
		buildPaths("loader", LOADER_PATHS)),
	sim_plugger(OTAWA_SIMULATOR_NAME, OTAWA_SIMULATOR_VERSION, SIMULATOR_PATHS)
{
	AbstractIdentifier::init();
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
		elm::system::Plugger::Iterator plug(ilp_plugger);
		if(plug.ended())
			return 0;
		plugin = (ilp::ILPPlugin *)plug.plug();
	}
	
	// Find a plugin
	else {
		plugin = (ilp::ILPPlugin *)ilp_plugger.plug(name.toCString());
	}

	// Return a system
	if(!plugin) {
		cerr << "ERROR: " << ilp_plugger.lastErrorMessage() << "\n";
		return 0;
	}
	return plugin->newSystem();
}


/**
 * This property, passed to the load configuration, gives the name of the
 * entry function of the current task.
 */
Identifier<CString> TASK_ENTRY("otawa::task_entry", "main");


/**
 * Identifier of the property indicating the name (CString) of the platform to use.
 */	
Identifier<CString> PLATFORM_NAME("otawa::platform_name", "");


/**
 * Identifier of the property indicating a name (CString) of the loader to use..
 */	
Identifier<CString> LOADER_NAME("otawa::loader_name", "");


/**
 * Identifier of the property indicating a platform (Platform *) to use.
 */	
Identifier<hard::Platform *> PLATFORM("otawa::platform", 0);


/**
 * Identifier of the property indicating the loader to use.
 */	
Identifier<Loader *> LOADER("otawa::loader", 0);


/**
 * Identifier of the property indicating the identifier (PlatformId) of the loader to use.
 */	
Identifier<hard::Platform::Identification *>
	PLATFORM_IDENTFIER("otawa::platform_identifier", 0);


/**
 * Argument count as passed to the program (int).
 */	
Identifier<int> ARGC("otawa::argc", -1);


/**
 * Argument values as passed to the program (char **).
 */	
Identifier<char **> ARGV("otawa::argv", 0);


/**
 * Argument values as passed to the program (char **).
 */	
Identifier<char **> ENVP("otawa::envp", 0);


/**
 * This property defines the used the used simulator when a simulator is
 * needed to perform simulation.
 */
Identifier<sim::Simulator *> SIMULATOR("otawa::simulator", 0);


/**
 * Name of the simulator to use.
 */
Identifier<elm::CString> SIMULATOR_NAME("otawa::simulator_name", "");


/**
 * This property is used to pass the cache configuration directly to the
 * platform.
 */
Identifier<hard::CacheConfiguration *>
	CACHE_CONFIG("otawa::cache_config", 0);


/**
 * This property is a hint to have an estimation of the pipeline depth.
 * It is better to look to the processor configuration in the patform.
 */
Identifier<int> PIPELINE_DEPTH("otawa::pipeline_depth", -1);


/**
 * This property shows that the system does not need to by simulated when
 * the binary image is built.
 */
Identifier<bool> NO_SYSTEM("otawa::no_system", false);


/**
 * This property shows that no stack need to be allocated.
 */
Identifier<bool> NO_STACK("otawa::no_stack", false);


/**
 * Path to the XML configuration file used in this computation.
 */
Identifier<elm::system::Path>
	CONFIG_PATH("otawa::config_path", "");


/**
 * XML element containing the configuration of the current computation.
 */
Identifier<elm::xom::Element *>
	CONFIG_ELEMENT("otawa::config_element", 0);


/**
 * Path to the XML configuration file of the processor.
 */
Identifier<elm::system::Path>
	PROCESSOR_PATH("otawa::processor_path", "");


/**
 * XML element containing the configuration of the processor.
 */
Identifier<elm::xom::Element *>
	PROCESSOR_ELEMENT("otawa::processor_element", 0);


/**
 * Gives the processor to use in the current computation.
 */
Identifier<hard::Processor *> PROCESSOR("otawa::processor", 0);


/**
 * Gives the path of file containing the cache configuration.
 */
Identifier<elm::system::Path> CACHE_CONFIG_PATH("otawa::cache_config_path", "");


/**
 * Gives an XML element containing the cache configuration.
 */
Identifier<elm::xom::Element *> CACHE_CONFIG_ELEMENT("otawa::cache_config_element", 0);


/**
 * Select the ILP solver plugin to use.
 */
Identifier<cstring> ILP_PLUGIN_NAME("otawa::ilp_plugin_name", "");


/**
 * Default manager. Avoid to declare one in the main.
 */
Manager MANAGER;  

}	// otawa
