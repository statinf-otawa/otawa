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

#include "../../config.h"
#include <elm/sys/System.h>
#include <elm/xom.h>
#include <elm/xom/XSLTransform.h>
#include <gel++.h>
#include <otawa/ilp/ILPPlugin.h>
#include <otawa/manager.h>
#include <otawa/platform.h>
#include <otawa/prog/Loader.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/sim/Simulator.h>
#include <otawa/util/XSLTScript.h>

using namespace elm;

namespace otawa {

// Private
String Manager::buildPaths(cstring kind, string paths) {
	StringBuffer buf;
	buf << "./.otawa/" << kind << elm::sys::Path::PATH_SEPARATOR
		<< elm::sys::Path::home() << "/.otawa/" << kind << elm::sys::Path::PATH_SEPARATOR;
#	ifdef HAS_RELOCATION
		buf << (Manager::prefixPath() / "lib/otawa" / kind) << elm::sys::Path::PATH_SEPARATOR;
		if(kind == "proc")
			buf << (Manager::prefixPath() / "lib/otawa") << elm::sys::Path::PATH_SEPARATOR;
#	endif
	buf << paths;
	return buf.toString();
}

// environment variable activating verbosity
cstring VERBOSE_ENV = "OTAWA_VERBOSE";


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
 * Name of the XML element containing the memory configuration.
 */
const CString Manager::MEMORY_NAME = "memory";


/**
 * Identifier used for Manager::load() configuration.
 * For complex loading ISA, several properties with such an identifier
 * may be put in the configuration property list. Its arguments is a string
 * of the form ID=VALUE where ID is an identifier of the XXX-config.xml
 * of the loader and VALUE the matching value.
 * @ingroup prog
 */
p::id<string> LOAD_PARAM("otawa::LOAD_PARAM", "");


/**
 * @class LoadException
 * Exception thrown when a loader encounters an error during load.
 */


/**
 * Build a load exception with a formatted message.
 * @param message	Message.
 */
LoadException::LoadException(const String& message)
: otawa::Exception(message) {
}


/**
 * @class Manager
 * The manager class providesfacilities for storing, grouping and retrieving shared
 * resources like loaders and platforms.
 *
 * @ingroup prog
 */


// test if verbose mode is activated
bool Manager::isVerbose(void) {
	return verbose == 1;
}


// reset verbosity at end of an action
void Manager::resetVerbosity(void) {
	verbose = -1;
}


// look for verbosity activation
void Manager::setVerbosity(const PropList& props) {

	// already set ?
	if(verbose != -1)
		return;

	// look in properties
	if(props.hasProp(Processor::VERBOSE)) {
		if(Processor::VERBOSE(props))
			verbose = 1;
		else
			verbose = 0;
		return;
	}

	// look in environment
	if(elm::sys::System::hasEnv(VERBOSE_ENV))
		verbose = 1;
	else
		verbose = 0;
}

#define CSTR(x) #x

/**
 * Manager builder. Install the PPC GLISS loader.
 */
Manager::Manager(void):
	ilp_plugger("ilp_plugin", Version(1, 0, 0),
		buildPaths("ilp", ILP_PATHS)),
	loader_plugger(OTAWA_LOADER_NAME, OTAWA_LOADER_VERSION,
		buildPaths("loader", LOADER_PATHS)),
	sim_plugger(OTAWA_SIMULATOR_NAME, OTAWA_SIMULATOR_VERSION, CSTR(SIMULATOR_PATHS))
{
	resetVerbosity();
}


/**
 * Delete all used resources.
 */
Manager::~Manager(void) {
	for(int i = 0; i < platforms.length(); i++)
		delete platforms[i];
}


/**
 * Find the loader matching the given name.
 * @param name	Name of the loader to find.
 * @return				Found loader or null.
 * @note This function should also perform dynamic loading of shared
 * library.
 */
Loader *Manager::findLoader(string name) {
	return (Loader *)loader_plugger.plug(name);
}


/**
 * Find the simulator matching the given name.
 * @param name	Name of the simulator to load.
 */
sim::Simulator *Manager::findSimulator(string name) {
	return (sim::Simulator *)sim_plugger.plug(name);
}


/**
 * Retrieve the file XXX-config.xml matching the path of the given
 * binary. XXX is usually the name of the instruction set.
 * @param	Path to the binary.
 * @return	Path to the configuration file if any, false else.
 */
elm::sys::Path Manager::retrieveConfig(const elm::sys::Path& path) {

	// retrieve the loader
	Loader *loader = findFileLoader(path);
	if(!loader)
		return "";

	// build the path of the configuration file
	Path cpath = loader->path().parent() / (loader->name() + "-config.xml");
	if(cpath.exists())
		return cpath;
	else
		return "";
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
 * @li @ref LOAD_PARAM,
 * @li @ref LOADER,
 * @li @ref LOADER_NAME,
 * @li @ref NO_STACK,
 * @li @ref NO_SYSTEM,
 * @li @ref MEMORY_ELEMENT
 * @li @ref MEMORY_OBJECT
 * @li @ref MEMORY_PATH
 * @li @ref PIPELINE_DEPTH,
 * @li @ref PLATFORM,
 * @li @ref PLATFORM_NAME,
 * @li @ref PROCESSOR,
 * @li @ref PROCESSOR_ELEMENT,
 * @li @ref PROCESSOR_PATH,
 * @li @ref SIMULATOR,
 * @li @ref SIMULATOR_NAME,
 * @li @ref TASK_ADDRESS,
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
WorkSpace *Manager::load(const elm::sys::Path&  path, const PropList& props) {
	setVerbosity(props);

	// Just load binary ?
	if(path.extension() != "xml")
		return loadBin(path, props);

	// Load the XML file
	loadXML(path, props);
	return load(CONFIG_ELEMENT(props), props);
}


/**
 * Copyright of OTAWA.
 */
CString Manager::copyright = "Copyright (c) 2016, University of Toulouse";


/**
 * Look for loader of the given path.
 * @param path	Path to the looked file.
 * @return		Found loader or NULL.
 */
Loader *Manager::findFileLoader(const elm::sys::Path& path) {
	Output log(io::err);
	
	// get the machine
	int mach;
	try {
		auto file = gel::Manager::open(path);
		mach = file->elfMachine();
		delete file;
	}
	catch(gel::Exception& e) {
		resetVerbosity();
		throw LoadException(e.message());		
	}
	
	// look for the plug-in
	string name = _ << "elf_" << mach;
	if(isVerbose()) {
		log << "INFO: looking for loader \"" << path << "\"\n";
		log << "INFO: searchpaths:\n";
		for(elm::sys::Plugger::PathIterator path(loader_plugger); path(); path++)
			log << "INFO:	- " << *path << io::endl;
		log << "INFO: available loaders\n";
		for(elm::sys::Plugger::Iter plugin(loader_plugger); plugin(); plugin++)
			log << "INFO:\t- " << *plugin << " (" << plugin.path() << ")\n";
	}
	return findLoader(name.toCString());
}


/**
 * Load a binary file.
 * @param path	Path to the binary file.
 * @param props	Configuration properties.
 * @return		Built framework.
 * @throws	LoadException	Error during load.
 */
WorkSpace *Manager::loadBin(
	const elm::sys::Path& path,
	const PropList& props)
{
	PropList used_props(props);
	setVerbosity(props);

	// get log
	io::OutStream *log_stream = Processor::LOG(props);
	if(!log_stream)
		log_stream = &io::err;
	Output log(*log_stream);

	// simple identified loader
	Loader *loader = LOADER(props);
	if(loader == nullptr) {
		auto name = LOADER_NAME(props);
		if(name) {
			if(isVerbose())
				log << "INFO: got loader name \"" << name << "\": ";
			loader = findLoader(name.toCString());
			if(isVerbose())
				log << (loader ? "found" : "not found") << io::endl;
		}
	}
	else if(isVerbose())
		log << "INFO: got loader from proplist: " << loader << io::endl;

	// try with gel
	if(loader == nullptr) {
		int mach;
		try {
			auto file = gel::Manager::open(path);
			mach = file->elfMachine();
			delete file;
		}
		catch(gel::Exception& e) {
			resetVerbosity();
			throw LoadException(e.message());			
		}
		string name = _ << "elf_" << mach;
		if(isVerbose()) {
			log << "INFO: looking for loader \"" << name << "\"\n";
			log << "INFO: prefix path = " << prefixPath() << io::endl;
			log << "INFO: searchpaths:\n";
			for(elm::sys::Plugger::PathIterator path(loader_plugger); path(); path++)
				log << "INFO:	- " << *path << io::endl;
			log << "INFO: available loaders\n";
			for(elm::sys::Plugger::Iter plugin(loader_plugger); plugin(); plugin++)
				log << "INFO:\t- " << *plugin << " (" << plugin.path() << ")\n";
		}
		loader = findLoader(name.toCString());
	}

	// post-process
	if(loader && isVerbose())
		cerr << "INFO: selected loader: " << loader->name() << " (" << loader->pluginVersion() << ") ( " << loader->path() << ")\n";
	if(!loader) {
		resetVerbosity();
		throw LoadException(_ << "no loader for \"" << path << "\".");
	}

	// any configuration ?
	Path cpath = loader->path().parent() / (loader->name() + "-config.xml");
	if(cpath.exists()) {
		/*if(isVerbose())
			cerr << "INFO: starting evaluation of load configuration file\n";
		XSLTScript script(cpath, elm::monitor, isVerbose());
		script.setConfiguration();
		script.addParams(props, LOAD_PARAM);
		script.transform();
		script.fillProps(used_props, "load");
		if(isVerbose())
			cerr << "INFO: ended evaluation of load configuration file\n";*/
	}

	// Try to load the binary
	resetVerbosity();
	return new WorkSpace(loader->load(this, path.asSysString(), used_props));
}


/**
 * Load an XML configuration file.
 * @param path	Path of the XML file.
 * @param props	Property to install the configuration in.
 * @return		Loaded workspace.
 * @throws	LoadException	Error during load.
 */
WorkSpace *Manager::loadXML(
	const elm::sys::Path& path,
	const PropList& props)
{
	setVerbosity(props);

	// Load the file
	xom::Builder builder;
	xom::Document *doc = builder.build(path.asSysString());
	if(!doc) {
		resetVerbosity();
		throw LoadException(_ << "cannot load \"" << path << "\".");
	}
	xom::Element *elem = doc->getRootElement();

	// Check the file
	if(elem->getLocalName() != "otawa"
	|| elem->getNamespaceURI() != OTAWA_NS) {
		resetVerbosity();
		throw LoadException("not a valid OTAWA XML.");
	}

	// Record the configuration
	if(isVerbose())
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
	ASSERT(elem);
	xom::Element *bin = elem->getFirstChildElement("binary");
	if(!bin) {
		resetVerbosity();
		throw LoadException("no binary available.");
	}
	elm::sys::Path bin_path = (CString)bin->getAttributeValue("ref");
	if(!bin_path) {
		resetVerbosity();
		throw LoadException("no binary available.");
	}
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
	elm::sys::Path path = CONFIG_PATH(props);
	if(path)
		return load(path, props);

	// Nothing to do
	resetVerbosity();
	throw LoadException("nothing to do.");
}


/**
 * Make an ILP system from the given plugin or from a named plugin.
 * @param name	Name of the plugin to use or an empty string for the
 * default plugin.
 * @param max	True to get a maximized system, false for a minimized one.
 * @return		A new ILP system ready to use or null (plugin not available).
 */
ilp::System *Manager::newILPSystem(string name, bool max) {
	ilp::ILPPlugin *plugin;

	// select "default" if required
	if(!name)
		name = "default";

	// try to open named
	plugin = static_cast<ilp::ILPPlugin *>(ilp_plugger.plug(name));

	// if not found, look the list of available
	if(!plugin) {
		elm::sys::Plugger::Iter plug(ilp_plugger);
		if(!plug.ended())
			plugin = static_cast<ilp::ILPPlugin *>(plug.plug());
	}

	// process error case
	if(!plugin) {
		cerr << "ERROR: " << ilp_plugger.lastErrorMessage() << "\n";
		return 0;
	}
	else
		return plugin->newSystem(max);
}


/**
 * Get the prefix path of the OTAWA installation,
 * that is, the installation directory path that allows to retrieve other
 * file resources of OTAWA.
 * @return	Prefix path.
 */
  elm::sys::Path Manager::prefixPath(void) {
	static Path prefix;
	if(!prefix) {
	  elm::sys::Path upath = elm::sys::System::getUnitPath((void *)buildPaths);
		if(upath)
			prefix = upath.parent().parent();
	}
	return prefix;
}


/**
 * This property, passed to the load configuration, gives the name of the
 * entry function of the current task.
 * @ingroup prog
 */
p::id<string> TASK_ENTRY("otawa::TASK_ENTRY", "main");


/**
 * This property, passed to the load configuration, select the task entry
 * by its address.
 * @ingroup prog
 */
p::id<Address> TASK_ADDRESS("otawa::TASK_ADDRESS", Address::null);


/**
 * Identifier of the property indicating the name (CString) of the platform to use.
 * @ingroup prog
 */
p::id<string> PLATFORM_NAME("otawa::PLATFORM_NAME", "");


/**
 * Identifier of the property indicating a name (CString) of the loader to use..
 * @ingroup prog
 */
p::id<string> LOADER_NAME("otawa::LOADER_NAME", "");


/**
 * Identifier of the property indicating a platform (Platform *) to use.
 * @ingroup prog
 */
p::id<hard::Platform *> PLATFORM("otawa::PLATFORM", 0);


/**
 * Identifier of the property indicating the loader to use.
 * @ingroup prog
 */
p::id<Loader *> LOADER("otawa::LOADER", 0);


/**
 * Identifier of the property indicating the identifier (PlatformId) of the loader to use.
 * @ingroup prog
 */
p::id<hard::Platform::Identification *>
	PLATFORM_IDENTFIER("otawa::PLATFORM_IDENTFIER", 0);


/**
 * Argument count as passed to the program (int).
 * @ingroup prog
 */
p::id<int> ARGC("otawa::ARGC", -1);


/**
 * Argument values as passed to the program (char **).
 * @ingroup prog
 */
p::id<char **> ARGV("otawa::ARGV", 0);


/**
 * Argument values as passed to the program (char **).
 * @ingroup prog
 */
p::id<char **> ENVP("otawa::ENVP", 0);


/**
 * This property defines the used the used simulator when a simulator is
 * needed to perform simulation.
 * @ingroup prog
 */
p::id<sim::Simulator *> SIMULATOR("otawa::SIMULATOR", 0);


/**
 * Name of the simulator to use.
 * @ingroup prog
 */
p::id<string> SIMULATOR_NAME("otawa::SIMULATOR_NAME", "");


/**
 * This property is used to pass the cache configuration directly to the
 * platform.
 * @ingroup prog
 */
p::id<hard::CacheConfiguration *>
	CACHE_CONFIG("otawa::CACHE_CONFIG", 0);


/**
 * This property is a hint to have an estimation of the pipeline depth.
 * It is better to look to the processor configuration in the patform.
 */
p::id<int> PIPELINE_DEPTH("otawa::PIPELINE_DEPTH", -1);


/**
 * This property shows that the system does not need to by simulated when
 * the binary image is built.
 * @ingroup prog
 */
p::id<bool> NO_SYSTEM("otawa::NO_SYSTEM", false);


/**
 * This property shows that no stack need to be allocated.
 * @ingroup prog
 */
p::id<bool> NO_STACK("otawa::NO_STACK", false);


/**
 * Path to the XML configuration file used in this computation.
 * @ingroup prog
 */
p::id<elm::sys::Path>
	CONFIG_PATH("otawa::CONFIG_PATH", "");


/**
 * XML element containing the configuration of the current computation.
 * @ingroup prog
 */
p::id<elm::xom::Element *>
	CONFIG_ELEMENT("otawa::CONFIG_ELEMENT", 0);


/**
 * Path to the XML configuration file of the processor.
 * @ingroup prog
 */
p::id<elm::sys::Path>
	PROCESSOR_PATH("otawa::PROCESSOR_PATH", "");


/**
 * XML element containing the configuration of the processor.
 * @ingroup prog
 */
p::id<elm::xom::Element *>
	PROCESSOR_ELEMENT("otawa::PROCESSOR_ELEMENT", 0);


/**
 * Gives the processor to use in the current computation.
 * @ingroup prog
 */
p::id<hard::Processor *> PROCESSOR("otawa::PROCESSOR", 0);


/**
 * Gives the path of file containing the cache configuration.
 * @ingroup prog
 */
p::id<elm::sys::Path> CACHE_CONFIG_PATH("otawa::CACHE_CONFIG_PATH", "");


/**
 * Gives an XML element containing the cache configuration.
 * @ingroup prog
 */
p::id<elm::xom::Element *> CACHE_CONFIG_ELEMENT("otawa::CACHE_CONFIG_ELEMENT", 0);


/**
 * this property may be used to pass information about the non-returning
 * behaviour of some functions. The parameter is the name of the function.
 * It is often used with the _exit function.
 * @ingroup prog
 */
p::id<string> NO_RETURN_FUNCTION("otawa::NO_RETURN_FUNCTION", "");


/**
 * Passed to Manager::load() to give the path of the file describing the memory.
 * @ingroup prog
 */
p::id<elm::sys::Path> MEMORY_PATH("otawa::MEMORY_PATH", "");


/**
 * Passed to manager::load() to give the XML element describing the memory configuration.
 * This property is often used with an XML file containing different configurations.
 * @ingroup prog
 */
p::id<elm::xom::Element *> MEMORY_ELEMENT("otawa::MEMORY_ELEMENT", 0);


/**
 * Passed to manager::load() to set the memory configuration.
 * The user is responsible of the release of the memory occipied by the memory object.
 * @ingroup prog
 */
p::id<hard::Memory *> MEMORY_OBJECT("otawa::MEMORY_OBJECT", 0);


/**
 * Default manager. Avoid to declare one in the main.
 * @ingroup prog
 */
Manager Manager::_def;


/**
 * @fn Manager& Manager::def(void);
 * Get the default manager.
 */


/**
 * Default manager. Avoid to declare one in the main.
 */
Manager& MANAGER = *Manager::def();


/**
 * Compilation date of this OTAWA library.
 */
const cstring Manager::COMPILATION_DATE = OTAWA_DATE;


/**
 * Current version of sources OTAWA (VCS version).
 */
const cstring Manager::VERSION = OTAWA_VERSION;


}	// otawa
