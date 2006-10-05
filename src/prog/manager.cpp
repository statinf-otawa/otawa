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
#include <elm/xom.h>
#include <otawa/sim/Simulator.h>

using namespace elm;

namespace otawa {


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
 * @class LoadException
 * Exception thrown when a loader encounters an error during load.
 */


/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param args		Arguments of the format.
 */
LoadException::LoadException(CString format, VarArg& args)
: MessageException(format, args) {
}


/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
LoadException::LoadException(CString format, ...) {
	VARARG_BEGIN(args, format)
		buildMessage(format, args);
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
 * Find a platform matching the given name.
 * @param name	Name of the platform to find.
 * @return				Found platform or null.
 */
/*hard::Platform *Manager::findPlatform(CString name) {
	return findPlatform(hard::Platform::Identification(name));
}*/

/**
 * Find a platform matching the given platform identifier.
 * @param id	Identifier of the platform.
 * @result			Found platform or null.
 */
/*hard::Platform *Manager::findPlatform(const hard::Platform::Identification& id) {
	for(int i = 0; i < platforms.count(); i++)
		if(platforms[i]->accept(id))
			return platforms[i];
	return 0;
}*/

/**
 * Load a file with the given path and the given properties.
 * @param path		Path of the file to load.
 * @param props		Properties describing the load process. It may contains the
 * properties : @ref TASK_ENTRY, @ref PLATFORM, @ref LOADER, @ref PLATFORM_NAME,
 * @ref LOADER_NAME, @ref ARGC, @ref ARGV, @ref ENVP, @ref SIMULATOR,
 * @ref CACHE_CONFIG, @ref PIPELINE_DEPTH.
 * @return The loaded framework or 0.
 */
FrameWork *Manager::load(const elm::system::Path&  path, PropList& props) {
	Process *proc = 0;
	
	// Is it a configuration XML file ?
	if(path.extension() == "xml") {
		/*xom::Builder builder;
		xom::Document *doc = builder.build(path);
		if(!doc)
			throw LoadException("cannot load \"%s\".", &path);
		xom::Element *elem = doc->getRootElement();
		if(elem->getLocalName() != "otawa")*/
		assert(0);
	}
	
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
		return new FrameWork(loader->load(this, &path, props));
}

/**
 * Manager builder. Install the PPC GLISS loader.
 */
Manager::Manager(void):
	ilp_plugger("ilp_plugin", Version(1, 0, 0), ILP_PATHS),
	loader_plugger(OTAWA_LOADER_NAME, OTAWA_LOADER_VERSION, LOADER_PATHS),
	sim_plugger(OTAWA_SIMULATOR_NAME, OTAWA_SIMULATOR_VERSION, SIMULATOR_PATHS)
{
	Identifier::init();
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
		if(!plugin) {
			cerr << "ERROR: " << ilp_plugger.lastErrorMessage() << "\n";
			return 0;
		}
	}

	// Return a system
	return plugin->newSystem();
}

/**
 * This property, passed to the load configuration, gives the name of the
 * entry function of the current task.
 */
GenericIdentifier<CString> TASK_ENTRY("task_entry", "main", OTAWA_NS);


/**
 * Identifier of the property indicating the name (CString) of the platform to use.
 */	
GenericIdentifier<CString> PLATFORM_NAME("platform_name", "", OTAWA_NS);


/**
 * Identifier of the property indicating a name (CString) of the loader to use..
 */	
GenericIdentifier<CString> LOADER_NAME("laoder_name", "", OTAWA_NS);


/**
 * Identifier of the property indicating a platform (Platform *) to use.
 */	
GenericIdentifier<hard::Platform *> PLATFORM("platform", 0, OTAWA_NS);


/**
 * Identifier of the property indicating the loader to use.
 */	
GenericIdentifier<Loader *> LOADER("loader", 0, OTAWA_NS);


/**
 * Identifier of the property indicating the identifier (PlatformId) of the loader to use.
 */	
GenericIdentifier<hard::Platform::Identification *>
	PLATFORM_IDENTFIER("platform_identifier", 0, OTAWA_NS);


/**
 * Argument count as passed to the program (int).
 */	
GenericIdentifier<int> ARGC("argc", -1, OTAWA_NS);


/**
 * Argument values as passed to the program (char **).
 */	
GenericIdentifier<char **> ARGV("argv", 0, OTAWA_NS);


/**
 * Argument values as passed to the program (char **).
 */	
GenericIdentifier<char **> ENVP("envp", 0, OTAWA_NS);


/**
 * This property defines the used the used simulator when a simulator is
 * needed to perform simulation.
 */
GenericIdentifier<sim::Simulator *> SIMULATOR("simulator", 0, OTAWA_NS);


/**
 * Name of the simulator to use.
 */
GenericIdentifier<elm::CString> SIMULATOR_NAME("simulator_name", "", OTAWA_NS);


/**
 * This property is used to pass the cache configuration directly to the
 * platform.
 */
GenericIdentifier<hard::CacheConfiguration *>
	CACHE_CONFIG("cache_config", 0, OTAWA_NS);


/**
 * This property is a hint to have an estimation of the pipeline depth.
 * It is better to look to the processor configuration in the patform.
 */
GenericIdentifier<int> PIPELINE_DEPTH("pipeline_depth", -1, OTAWA_NS);


/**
 * This property shows that the system does not need to by simulated when
 * the binary image is built.
 */
GenericIdentifier<bool> NO_SYSTEM("no_system", false, OTAWA_NS);


/**
 * Path to the XML configuration file used in this computation.
 */
GenericIdentifier<elm::system::Path>
	CONFIG_PATH("config_path", "", OTAWA_NS);


/**
 * XML element containing the configuration of the current computation.
 */
GenericIdentifier<elm::xom::Element *>
	CONFIG_ELEMENT("config_element", 0, OTAWA_NS);


/**
 * Path to the XML configuration file of the processor.
 */
GenericIdentifier<elm::system::Path>
	PROCESSOR_PATH("processor_path", "", OTAWA_NS);


/**
 * XML element containing the configuration of the processor.
 */
GenericIdentifier<elm::xom::Element *>
	PROCESSOR_ELEMENT("processor_element", 0, OTAWA_NS);


/**
 * Gives the processor to use in the current computation.
 */
GenericIdentifier<hard::Processor *> PROCESSOR("processor", 0, OTAWA_NS);

}	// otawa
