/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	Registry class implementation
 */

#include <otawa/proc/Registry.h>

namespace otawa {

/**
 * Class dedicated to the registering of the processors.
 */


/**
 * Default registry.
 */
Registry Registry::_;


/**
 * Look for a processor matching the given name.
 * @param name	Name of the processor to look for.
 * @return		Found processor or null.
 */
const Registration *Registry::find(CString name) {
	return _.procs.get(name);
}


/**
 * To ensure that no other registry is built.
 */
Registry::Registry(void): Initializer<Registration>(false) {
	startup();
}


/**
 * @class Registration
 * Abstract class to represent the registered processors.
 * 
 * To build a full registration for a processor, we need:
 * @li a processor piece (the default processor value),
 * @li a list of configuration (identifier allowing the configuation of the
 * processor),
 * @li and the registration itself.
 * Note that the configuration objects must registered in the processor
 * constructor using the ''config()'' protected method.
 * 
 * Below is an example of registration of the @ref WCETComputation.
 * @code
 * #include <otawa/proc/Registry.h>
 * #include <otawa/ipet/WCETComputation.h>
 * 
 * using namespace otawa;
 * using namespace otawa::ipet;
 * 
 * static Configuration verbose(VERBOSE, DOC_PATH "/verbose.html");
 * static WCETComputation proc;
 * static Registration reg(
 * 		proc,
 * 		DOC_PATH "/WCETComputation.html");
 * 
 * WCETComputation::WCETComputation(void) {
 * 		...
 * 		config(verbose)
 * }
 * @endcode
 */


/**
 * Build a processor registering.
 * @param processor	Default processor value.
 * @param help		URL to an help document.
 */
Registration::Registration(Processor& processor, CString help)
: _processor(&processor), _help(help) {
	Registry::_.record(this);
}


/**
 * @fn String Registration::name(void) const;
 * Get the name of the processor.
 * @return	Processor name.
 */


/**
 * @fn Version Registration::version(void) const;
 * Get the version of the processor.
 * @return	Processor version.
 */


/**
 * @fn Processor *Registration::processor(void) const;
 * Get the default processor.
 * @return	Default processor.
 */


/**
 * @fn CString Registration::help(void) const;
 * Get the URL of a help document.
 * @return	Help document URL.
 */


/**
 * Internal.
 */
void Registration::initialize(void) {
	Registry::_.procs.put(name(), this);
}


/**
 * @class Configuration
 * Class to record a configuration identifier for a processor.  
 */


/**
 * @fn Configuration::Configuration(AbstractIdentifier& id, CString help);
 * Build a configuration.
 * @param id	Used identifier.
 * @param help	URL to get an help message.
 */


/**
 * @fn AbstractIdentifier& Configuration::id(void) const;
 * Get the identifier of this configuration.
 */


/**
 * @fn CString Configuration::help(void) const;
 * Get the help URL of this configuration.
 */


/**
 * @class Registration::ConfigIter
 * Iterator on the configurations of a processor.
 */


/**
 * @fn Registration::ConfigIterConfigIter(Registration& reg);
 * Build the configuration iterator.
 * @param reg	Registration to look configuration in.
 */

/**
 * @fn Registration::ConfigIterConfigIter(Registration *reg);
 * Build the configuration iterator.
 * @param reg	Registration to look configuration in.
 */

} // otawa
