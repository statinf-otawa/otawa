/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/proc_Processor.cpp -- Processor class implementation.
 */

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

using namespace elm;
using namespace elm::io;

namespace otawa {

/**
 * @class Processor
 * The processor class is implemented by all code processor. At this level,
 * the argument can only be the full framework. Look at sub-classes for more
 * accurate processors.
 * 
 * The processor configuration accepts the following properties:
 * @ref PROC_OUTPUT, @ref PROC_VERBOSE, @ref PROC_STATS, @ref PROC_TIMED.
 * 
 * This processor may generate the following statistics: @ref PROC_RUNTIME.
 */


/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param props		Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(elm::String name, elm::Version version,
const PropList& props): _name(name), _version(version), flags(0) {
	init(props);
}

/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 */
Processor::Processor(String name, Version version)
: _name(name), _version(version), flags(0) {
}


/**
 * Build a new processor.
 * @param	Configuration properties.
 */
Processor::Processor(const PropList& props): flags(0) {
	init(props);
}


/**
 */
void Processor::init(const PropList& props) {
	
	// Process output
	OutStream *out_stream = PROC_OUTPUT(props);
	if(out_stream)
		out.setStream(*out_stream);
		
	// Process statistics
	stats = PROC_STATS(props);
	if(stats) { 
		if(PROC_TIMED(props))
			flags |= TIMED;
		else
			flags &= ~TIMED;
	}
	
	// Process verbosity
	if(PROC_VERBOSE(props))
		flags |= VERBOSE;
	else
		flags &= ~VERBOSE;
}


/**
 * @fn void Processor::processFrameWork(FrameWork *fw);
 * Process the given framework.
 * @param fw	Framework to process.
 */


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
 * @param fw	Framework to work on.
 * @param props	Configuration properties.
 */
void Processor::process(FrameWork *fw, const PropList& props) {
	
	// Check required feature
	for(int i = 0; i < required.length(); i++)
		if(!fw->isProvided(*required[i]))
			required[i]->process(fw, props);

	// Perform configuration
	configure(props);

	// Pre-processing actions
	if(isVerbose())
		out << "Starting " << name() << " (" << version() << ')' << io::endl;
	system::StopWatch swatch;
	if(isTimed())
		swatch.start();
	
	// Launch the work
	processFrameWork(fw);
	
	// Post-processing actions
	if(isVerbose())
		out << "Ending " << name();
	if(isTimed()) {
		swatch.stop();
		PROC_RUNTIME(*stats) = swatch.delay();
		if(isVerbose())
			out << " (" << (swatch.delay() / 1000) << "ms)" << io::endl;
	}
	if(isVerbose())
		out << io::endl;
	
	// Add provided features
	for(int i = 0; i < provided.length(); i++)
		fw->provide(*provided[i]);
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
 * This property identifier is used for setting the output stream used by
 * the processor for writing messages (information, warning, error) to the user.
 */
GenericIdentifier<elm::io::OutStream *> PROC_OUTPUT("otawa.proc.output", 0);


/**
 * This property identifiers is used to pass a property list to the processor
 * that will be used to store statistics about the performed work. Implicitly,
 * passing such a property activates the statistics recording facilities.
 */
GenericIdentifier<PropList *> PROC_STATS("otawa.proc.stats", 0);


/**
 * If the value of the associated property is true (default to false), time
 * statistics will also be collected with other processor statistics. Passing
 * such a property without @ref PROC_STATS has no effects.
 */
GenericIdentifier<bool> PROC_TIMED("otawa.proc.timed", false);


/**
 * This property identifier is used to store in the statistics of a processor
 * the overall run time of the processor work.
 */
GenericIdentifier<elm::system::time_t> PROC_RUNTIME("otawa.proc.runtime", 0);


/**
 * This property activates the verbose mode of the processor: information about
 * the processor work will be displayed.
 */
GenericIdentifier<bool> PROC_VERBOSE("otawa.proc.verbose", false);


/**
 * This property selects the task entry point currently processed.
 */
GenericIdentifier<elm::CString> PROC_ENTRY("otawa.proc.entry", "main");


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
 * provided by the work of the current processor.
 * @param feature	Provided feature.
 */
void Processor::provide(const AbstractFeature& feature) {
	provided.add(&feature);
}

} // otawa
