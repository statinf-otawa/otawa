/*
 *	$Id$
 *	Processor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-8, IRIT UPS.
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
#include <otawa/proc/FeatureDependency.h>

using namespace elm;
using namespace elm::io;

/**
 * @defgroup proc Processor Layer
 * The OTAWA @ref prop provides a flexible and easy way to store in the program representation
 * the result of different analyses. Yet, the computation of the WCET may require a lot
 * of different analyses, it becomes wuicly difficult to handle the chaining of different
 * analyses according their requirement.
 */

/**
 * @page proc_concepts Processor Concepts
 * @ingroup proc
 *
 * @par Features
 *
 * A feature represents a set of services that are available on the program representation.
 * These services are provided either by the program loader, or by a previous analysis.
 * These services are composed of:
 * @li properties linked to the program representation,
 * @li methods providing meaninful results in the objefct of the program representation.
 *
 * The feature are an easier way to inform the framework that some services are provided
 * and to group properties and methods in logical sets. A feature is mainly implemented
 * by an @ref otawa::AbstractFeature.
 *
 * It may be implemented by a simple @ref otawa::Feature with a default code processor
 * (see below) to satisfy the service if it is not provided. As the goal of feature is
 * to share some data between analyses, it is first declared in a header file, for
 * example "my_feature.h" :
 * @code
 * #include <otawa/proc/Feature.h>
 * #include "MyDefaultProcessor.h"
 *
 * extern Feature<MyDefaultProcessor> MY_FEATURE;
 * @endcode
 *
 * Then, it must defined in a source file, for example, "my_feature.cpp":
 * @code
 * #include "my_feature.h"
 *
 * Feature<MyDefaultProcessor> MY_FEATURE("MY_FEATURE");
 * @endcode
 *
 * Note that a feature must have a unique name: duplicate names in features names will be
 * detected at system startup and will cause abortion of the application. Although it is
 * not mandatory, it is advised (1) to give a name in uppercase with underscores to separe
 * names and (2) to give the full-qualified C++ name. This last property will help to
 * retrieve features when dynamic module loading is used.
 *
 * The way proposed above to declare a feature has a drawback: it is costly in computation
 * time as it requires the user to include the default processor class.
 * To avoid this, OTAWA propose to use the  @ref otawa::SilentFeature class.
 * In this cas, the header file will become:
 * @code
 * #include <otawa/proc/SilentFeature.h>
 *
 * extern SilentFeature MY_FEATURE;
 * @endcode
 *
 * The, the source file is a bit more complex as it must now design the default processor:
 * @code
 * #include "my_feature.h"
 * #include "MyDefaultProcessor"
 *
 * static SilentFeature::Maker<MyDefaultProcessor> MY_MAKER;
 * SilentFeature MY_FEATURE("MY_FEATURE", MY_MAKER);
 * @endcode
 *
 * @par Code Processors
 *
 * A code processor performs an analysis, that is, scan the program representation,
 * extracts some properties and provide them to the remaining analyses. To organize the
 * properties, they are grouped logically in features. To organize this work, the
 * code processors records themselves to OTAWA in order to:
 * @li ensures that the required features are available,
 * @li inform that the processor computes some new features.
 *
 * Consequently, in OTAWA, analyses are packaged inside code processors
 * (@ref otawa::Processor) that manages the features but also provides other
 * facilities:
 * @li configuration of the processor work,
 * @li control of output streams,
 * @li statistics gathering,
 * @li details about the computation
 * @li several classes to make analysis writing easier (CFG processor, BB processor,
 * contextual processor and so on),
 * @li resource cleanup.
 *
 * This class provides also a common way to invoke analyses. The configuration
 * is perform in a property list structure:
 * @code
 * PropList props;
 * CONFIGURATION_1(props) = value1;
 * CONFIGURATION_2(props) = value2;
 * ...
 * @endcode
 *
 * Then, the processor object is simply created and invoked on a workspace:
 * @code
 * MyProcessor proc;
 * proc.process(workspace, props);
 * @endcode
 *
 * For example, to redirect the log output of the processor and to activate
 * verbosity, and to compute the domination in a CFG, we may do:
 * @code
 * PropList props;
 * Processor::VERBOSE(props) = true;
 * Processor::LOG(props) = System::newFile("my_log.txt");
 * Domination dom;
 * dom.process(workspace, props);
 * @endcode
 *
 * Notice that the configuration properties are passed to all processor invoked to provided
 * features required by the invoked processor.
 */

namespace otawa {

// Registration
Processor::__init::__init(void) {
	Processor::init();
}
void Processor::init(void) {
	// workaround right bug in GCC 4.0
	// subclass of a friend class is not friend
	__reg._name = "otawa::Processor";
	__reg._version = Version(1, 1, 0);
	__reg.configs.add(&Processor::OUTPUT);
	__reg.configs.add(&Processor::LOG);
	__reg.configs.add(&Processor::VERBOSE);
	__reg.configs.add(&Processor::STATS);
	__reg.configs.add(&Processor::TIMED);
	__reg.record();
}
Processor::__init Processor::__reg;


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
 *
 * @ingroup proc
 */


/**
 * Build a simple anonymous processor.
 */
Processor::Processor(void): flags(0), stats(0) {
	reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	reg->_base = &__reg;
}


/**
 */
Processor::~Processor(void) {
	if(flags & IS_ALLOCATED)
		delete reg;
}


/**
 * For internal use only.
 */
Processor::Processor(AbstractRegistration& registration)
: flags(0), stats(0) {
	reg = &registration;
}


/**
 * For internal use only.
 */
Processor::Processor(String name, Version version, AbstractRegistration& registration)
: flags(0), stats(0) {
	reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	reg->_base = &registration;
	reg->_name = name;
	reg->_version = version;
}


/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param props		Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(elm::String name, elm::Version version,
const PropList& props): flags(0), stats(0) {
	reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	reg->_base = &__reg;
	reg->_name = name;
	reg->_version = version;
	init(props);
}

/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 */
Processor::Processor(String name, Version version)
: flags(0), stats(0) {
	reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	reg->_base = &__reg;
	reg->_name = name;
	reg->_version = version;
}


/**
 * Build a new processor.
 * @param			Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(const PropList& props): flags(0), stats(0) {
	reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	reg->_base = &__reg;
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

	// Process verbosity
	if(VERBOSE(props))
		flags |= IS_VERBOSE;
	else
		flags &= ~IS_VERBOSE;

	// Process timing
	if(isVerbose() || recordsStats()) {
		if(TIMED(props))
			flags |= IS_TIMED;
		else
			flags &= ~IS_TIMED;
	}
}


// Only for the deprecation warning
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

	// Perform configuration
	ws = fw;
	configure(props);

	// Remove non-used invalidated features
	for(FeatureIter feature(*reg); feature; feature++)
		if(feature->kind() == FeatureUsage::invalidate
		&& !reg->uses(feature->feature())) {
			if(isVerbose())
				log << "INVALIDATED: " << feature->feature().name()
					<< " by " << reg->name() << io::endl;
			fw->invalidate(feature->feature());
		}

	// Get used feature
	Vector<const AbstractFeature *> required;
	for(FeatureIter feature(*reg); !ws->isCancelled() && feature; feature++)
		if(feature->kind() == FeatureUsage::require
		|| feature->kind() == FeatureUsage::use) {
			if(feature->kind() == FeatureUsage::require)
				required.add(&feature->feature());
			if(isVerbose()) {
				cstring kind = "USED";
				if(feature->kind() == FeatureUsage::require)
					kind = "REQUIRED";
				log << kind << ": " << feature->feature().name()
					<< " by " << reg->name() << io::endl;
			}
			try {
				fw->require(feature->feature(), props);
			}
			catch(NoProcessorException& e) {
				throw UnavailableFeatureException(this, feature->feature());
			}
		}
	if(ws->isCancelled())
		return;

	// check before starting processor
	for(FeatureIter feature(*reg); feature; feature++)
		if((feature->kind() == FeatureUsage::require
		|| feature->kind() == FeatureUsage::use)
		&& !fw->isProvided(feature->feature()))
			throw otawa::Exception(_ << 'feature ' << feature->feature().name()
				<< " is not provided after one cycle of requirements:\n"
				<< "stopping -- this may denotes circular dependencies.");

	// Pre-processing actions
	if(isVerbose())
		log << "Starting " << name() << " (" << version() << ')' << io::endl;
	system::StopWatch swatch;
	if(isTimed())
		swatch.start();
	setup(fw);

	// Launch the work
	processWorkSpace(fw);

	// Post-processing actions
	cleanup(fw);
	if(isVerbose())
		log << "Ending " << name();
	if(isTimed()) {
		swatch.stop();
		if(recordsStats())
			RUNTIME(*stats) = swatch.delay();
		if(isVerbose())
			log << " (" << ((double)swatch.delay() / 1000) << "ms)" << io::endl;
	}
	if(isVerbose())
		log << io::endl;

	// Cleanup used invalidated features
	for(FeatureIter feature(*reg); feature; feature++)
		if(feature->kind() == FeatureUsage::invalidate
		&& reg->uses(feature->feature())) {
			if(isVerbose())
				log << "INVALIDATED: " << feature->feature().name()
					<< " by " << reg->name() << io::endl;
			fw->invalidate(feature->feature());
		}

	// Add provided features
	for(FeatureIter feature(*reg); feature; feature++)
		if(feature->kind() == FeatureUsage::provide) {
			if(isVerbose())
				log << "PROVIDED: " << feature->feature().name()
					<< " by " << reg->name() << io::endl;
			fw->provide(feature->feature(), &required);
		}

	// Put the cleaners
	for(clean_list_t::Iterator clean(cleaners); clean; clean++) {
		FeatureDependency *dep = ws->getDependency((*clean).fst);
		ASSERTP(dep, "cleanup invoked for a not provided feature: " + (*clean).fst->name());
		(*dep)((*clean).snd);
	}
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
	Processor::OUTPUT("otawa::Processor::OUTPUT", &io::stdout);


/**
 * This property identifier is used for setting the log stream used by
 * the processor to write messages (information, warning, error).
 */
Identifier<elm::io::OutStream *>
	Processor::LOG("otawa::Processor::LOG", &io::stderr);


/**
 * This property identifiers is used to pass a property list to the processor
 * that will be used to store statistics about the performed work. Implicitly,
 * passing such a property activates the statistics recording facilities.
 */
Identifier<PropList *> Processor::STATS("otawa::Processor::STATS", 0);


/**
 * If the value of the associated property is true (default to false), time
 * statistics will also be collected with other processor statistics. Passing
 * such a property without @ref PROC_STATS has no effects.
 */
Identifier<bool> Processor::TIMED("otawa::Processor::TIMED", false);


/**
 * This property identifier is used to store in the statistics of a processor
 * the overall run time of the processor work.
 */
Identifier<elm::system::time_t> Processor::RUNTIME("otawa::Processor::RUNTIME", 0);


/**
 * This property activates the verbose mode of the processor: information about
 * the processor work will be displayed.
 */
Identifier<bool> Processor::VERBOSE("otawa::Processor::VERBOSE", false);


/**
 * Usually called from a processor constructor, this method records a
 * required feature for the work of the current processor.
 * @param feature	Required feature.
 */
void Processor::require(const AbstractFeature& feature) {
	reg->features.add(FeatureUsage(FeatureUsage::require, feature));
}


/**
 * Usually called from a processor constructor, this method records a feature
 * invalidated by the work of the current processor.
 * @param feature	Invalidated feature.
 */
void Processor::invalidate(const AbstractFeature& feature) {
	reg->features.add(FeatureUsage(FeatureUsage::invalidate, feature));
}


/**
 * Usually called from a processor constructor, this method records a feature
 * as used by the work of the current processor.
 * @param feature	Used feature.
 */
void Processor::use(const AbstractFeature& feature) {
	reg->features.add(FeatureUsage(FeatureUsage::use, feature));
}


/**
 * Usually called from a processor constructor, this method records a feature
 * provided by the work of the current processor.
 * @param feature	Provided feature.
 */
void Processor::provide(const AbstractFeature& feature) {
	reg->features.add(FeatureUsage(FeatureUsage::provide, feature));
}


/**
 * @fn WorkSpace *Processor::workspace(void) const;
 * Get the current workspace.
 * @return	Current workspace.
 */


/**
 * @fn void Processor::addCleaner(const AbstractFeature& feature, Cleaner *cleaner);
 * Add a cleaner for the given feature.
 * @param feature	Feature the cleaner apply to.
 * @param cleaner	Cleaner to add.
 */


/**
 * @fn void Processor::track(AbstractFeature& feature, T *object);
 * Track the release of an allocated object with the given feature.
 * Ensure that the given object will be deleted when the feature is invalidated.
 * @param feature	Feature the cleaner applies to.
 * @param object	Object to delete.
 */


/**
 * @fn void Processor::track(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref);
 * Track the release of an allocated object assigned to an identifier relatively to the
 * given feature. When the feature is deleted, the object is fried and the identifier
 * is removed. It is used as below:
 * @code
 * 		track(MY_ID(props) = value);
 * @endcode
 *
 * @param feature	Linked feature.
 * @param ref		Reference to the identifier to remove.
 */


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
 * @ingroup proc
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
 * @ingroup proc
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
