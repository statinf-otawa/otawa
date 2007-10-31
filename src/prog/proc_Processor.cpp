/*
 *	$Id$
 *	Processor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-7, IRIT UPS.
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

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/Registry.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;
using namespace elm::io;

namespace otawa {

// Registration
static Configuration output_conf(Processor::OUTPUT, AUTODOC "/classotawa_1_1Processor.html");
static Configuration log_conf(Processor::LOG, AUTODOC "/classotawa_1_1Processor.html");
static Configuration verbose_conf(Processor::VERBOSE, AUTODOC "/classotawa_1_1Processor.html");
static Configuration stats_conf(Processor::STATS, AUTODOC "/classotawa_1_1Processor.html");
static Configuration timed_conf(Processor::TIMED, AUTODOC "/classotawa_1_1Processor.html");


/**
 * @class Processor
 * The processor class is implemented by all code processor. At this level,
 * the argument can only be the full framework. Look at sub-classes for more
 * accurate processors.
 * 
 * @p Configuration
 * @li @ref Processor::OUTPUT,
 * @li @ref Processor::LOG,
 * @li @ref Processor::VERBOSE,
 * @li @ref Processor::STATS,
 * @li @ref Processor::TIMED.
 * 
 * @p Statistics
 * The statistics are recorded in the property list passed by @ref Processor::STATS. 
 * @li @ref Processor::RUNTIME.
 */


/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param props		Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(elm::String name, elm::Version version,
const PropList& props): _name(name), _version(version), flags(0), stats(0){
	init(props);
	config(output_conf);
	config(log_conf);	
	config(verbose_conf);
	config(stats_conf);
	config(timed_conf);
}

/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 */
Processor::Processor(String name, Version version)
: _name(name), _version(version), flags(0), stats(0) {
	config(output_conf);
	config(verbose_conf);
	config(stats_conf);
	config(timed_conf);
}


/**
 * Build a new processor.
 * @param			Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(const PropList& props): flags(0), stats(0) {
	init(props);
}


/**
 */
void Processor::init(const PropList& props) {
	
	// Process output
	out.setStream(*OUTPUT(props));
	log.setStream(*LOG(props));
		
	// Process statistics
	stats = STATS(props);
	if(stats) { 
		if(TIMED(props))
			flags |= IS_TIMED;
		else
			flags &= ~IS_TIMED;
	}
	
	// Process verbosity
	if(VERBOSE(props))
		flags |= IS_VERBOSE;
	else
		flags &= ~IS_VERBOSE;
}


// Only for the deprecatio warning
static bool deprecated;

/**
 * Process the given workspace.
 * @param fw	Workspace to process.
 * @deprecated	Use @ref processWorkSpace() instead.
 */
void Processor::processFrameWork(WorkSpace *fw) {
	deprecated = false;
}


/**
 * Process the given framework.
 * @param fw	Framework to process.
 * @deprecated	Use @ref processWorkSpace() instead.
 */
void Processor::processWorkSpace(WorkSpace *fw) {
	
	// Only to keep compatility (03/05/07)
	deprecated = true;
	processFrameWork(fw);
	if(deprecated)
		warn(_ << "WARNING: the use of processFrameWork() is deprecated."
			<< "Use processWorkSpace() instead.");
}


/**
 * This method may be called for configuring a processor thanks to information
 * passed in the property list.
 * @param props	Configuration information.
 */
void Processor::configure(const PropList& props) {
	init(props);
}


/**
 * Execute the code processor on the given framework.
 * @param fw	Workspace to work on.
 * @param props	Configuration properties.
 */
void Processor::process(WorkSpace *fw, const PropList& props) {
	
	// Check required feature
	for(int i = 0; i < required.length(); i++) {
		try {
			fw->require(*required[i], props);				
		}
		catch(NoProcessorException& e) {
			throw UnavailableFeatureException(this, *required[i]);
		}
	}

	// Perform configuration
	configure(props);

	// Pre-processing actions

	if(isVerbose()) 
	log << "Starting " << name() << " (" << version() << ')' << io::endl;
	system::StopWatch swatch;
	if(isTimed())
		swatch.start();
	
	// Launch the work
	setup(fw);
	processWorkSpace(fw);
	cleanup(fw);
	
	// Post-processing actions
	if(isVerbose())
		log << "Ending " << name();
	if(isTimed()) {
		swatch.stop();
		RUNTIME(*stats) = swatch.delay();
		if(isVerbose()) 
			log << " (" << (swatch.delay() / 1000) << "ms)" << io::endl;
	}
	if(isVerbose()) 
		log << io::endl;
	
	// Remove invalidated features
	for (int i = 0; i < invalidated.length(); i++) {
		fw->invalidate(*invalidated[i]); // recursively invalidate all children
	}
	
	// Add provided features
	for(int i = 0; i < provided.length(); i++)
		fw->provide(*provided[i], &required);
}


/**
 * @fn bool Processor::isVerbose(void) const;
 * Test if the verbose mode is activated.
 * @return	True if the verbose is activated, false else.
 */


/**
 * @fn bool Processor::isTimed(void) const;
 * Test if the timed mode is activated (recording of timings in the statistics).
 * @return	True if timed mode is activated, false else.
 * @note	If statistics are not activated, this method returns ever false.
 */


/**
 * @fn bool Processor::recordsStats(void) const;
 * Test if the statictics mode is activated.
 * @return	True if the statistics mode is activated, false else.
 */


/**
 * This method is called before an anlysis to let the processor do some
 * initialization.
 * @param fw	Processed workspace.
 */
void Processor::setup(WorkSpace *fw) {
}


/**
 * This method is called after the end of the processor analysis to let it
 * do some clean up.
 * @param fw	Workspace to work on.
 */
void Processor::cleanup(WorkSpace *fw) {
}


/**
 * Display a warning.
 * @param message	Message to display.
 */
void Processor::warn(const String& message) {
	log << "WARNING:" << name() << ' ' << version()
		<< ':' << message << io::endl;
}


/**
 * @fn void Processor::config(Configuration& config);
 * Add the given configuration identifier to the list of configuration of this
 * processor.
 * @param config	Configuration identifier to add.
 */


/**
 * This property identifier is used for setting the output stream used by
 * the processor to write results.
 */
Identifier<elm::io::OutStream *>
	Processor::OUTPUT("otawa::Processor::output", &io::stdout);


/**
 * This property identifier is used for setting the log stream used by
 * the processor to write messages (information, warning, error).
 */
Identifier<elm::io::OutStream *>
	Processor::LOG("otawa::Processor::log", &io::stderr);


/**
 * This property identifiers is used to pass a property list to the processor
 * that will be used to store statistics about the performed work. Implicitly,
 * passing such a property activates the statistics recording facilities.
 */
Identifier<PropList *> Processor::STATS("otawa::Processor::stats", 0);


/**
 * If the value of the associated property is true (default to false), time
 * statistics will also be collected with other processor statistics. Passing
 * such a property without @ref PROC_STATS has no effects.
 */
Identifier<bool> Processor::TIMED("otawa::Processor::timed", false);


/**
 * This property identifier is used to store in the statistics of a processor
 * the overall run time of the processor work.
 */
Identifier<elm::system::time_t> Processor::RUNTIME("otawa::Processor::runtime", 0);


/**
 * This property activates the verbose mode of the processor: information about
 * the processor work will be displayed.
 */
Identifier<bool> Processor::VERBOSE("otawa::Processor::verbose", false);


/**
 * Usually called from a processor constructor, this method records a 
 * required feature for the work of the current processor.
 * @param feature	Required feature.
 */
void Processor::require(const AbstractFeature& feature) {
	required.add(&feature);
}


/**
 * Usually called from a processor constructor, this method records a feature
 * invalidated by the work of the current processor.
 * @param feature	Invalidated feature.
 */
void Processor::invalidate(const AbstractFeature& feature) {
	invalidated.add(&feature);
}

/**
 * Usually called from a processor constructor, this method records a feature
 * provided by the work of the current processor.
 * @param feature	Provided feature.
 */
void Processor::provide(const AbstractFeature& feature) {
	provided.add(&feature);
}



/**
 * @class NullProcessor
 * A simple processor that does nothing.
 */


/**
 */
NullProcessor::NullProcessor(void):
	Processor("otawa::NullProcessor", Version(1, 0, 0))
{
}


/**
 * @class NoProcessor class
 * A processor whise execution cause an exception throw. Useful for features
 * without default definition.
 */


/**
 */
void NoProcessor::processWorkSpace(WorkSpace *fw) {
	throw NoProcessorException();
}


/**
 */
NoProcessor::NoProcessor(void):
	Processor("otawa::NoProcessor", Version(1, 0, 0))
{
}


/**
 * @class UnavailableFeatureException
 * This exception is thrown when an feature can not be computed.
 */


/**
 * @fn UnavailableFeatureException::UnavailableFeatureException(const Processor *processor, const AbstractFeature& feature);
 * @param processor	Processor causing the exception.
 * @param feature	Feature causing the exception.
 */


/**
 * @fn const AbstractFeature& UnavailableFeatureException::feature(void) const;
 * Get the feature causing the exception.
 * @return	Involved feature.
 */


/**
 */
String UnavailableFeatureException::message(void) {
	return _ << ProcessorException::message()
			 << " requires the feature \"" << f.name() << "\".";
}

} // otawa
