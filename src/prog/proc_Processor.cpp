/*
 *	$Id$
 *	Processor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#include <elm/system/System.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/Registry.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/FeatureDependency.h>
#include <otawa/proc/Progress.h>
#include <otawa/stats/StatInfo.h>
#include <otawa/stats/StatCollector.h>

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
 *
 *
 * @par Processor Registration
 *
 * Code processors or analyzers can be invoked explicitly from C++ code, implicitly
 * by requiring a feature or from a script (see @ref Script class or @c owcet command).
 * In the latter case, the processor name is used to retrieve its code and to allow
 * to create an instance.
 *
 * As C++ does not provide any reflexive information to find back the constructor of a class,
 * the code processor need to record in the main registry of OTAWA. To achieve, it must
 * declare a static object (1) that will record itself to the main registry and (2) that
 * provides all details of the code processor to the OTAWA framework:
 * @li the name
 * @li the version
 * @li required and/or used features,
 * @li invalidated features,
 * @li provided features,
 * @li configuration properties,
 * @li the constructor,
 * @li possibly its base processor (as a default, all processor have for base otawa::Processor).
 *
 * Although this registration can be achieved in different ways, the current preferred way requires
 * to declare, as public members, a static attribute named @c reg and a constructor taking an AbstractRegistration
 * with @c reg as default value. The constructor definition allows to pass the actual registration of the subclass
 * (that is including the one of the current class) to the OTAWA framework.
 * @code
 * class MyProcessor: public BBProcessor {
 * public:
 * 		static p::declare reg;
 * 		MyProcessor(AbstractRegistration& r = reg): BBProcessor(r) { ... }
 * };
 * @endcode
 *
 * In turn, the @c static attribute must be declared with the details of the code processor
 * in the source file:
 * @code
 * p::declare MyProcessor::reg =
 * 	p::init("MyProcessor", Version(1, 0, 0))
 * 	.base(BBProcessor::reg),
 * 	.require(CFG_COLLECTED_FEATURE)
 * 	.provide(MY_FEATURE)
 * 	.make<MyProcessor>();
 * @endcode
 */

namespace otawa {

extern cstring VERBOSE_ENV;

/**
 * A registration to customize.
 */
class CustomRegistration: public AbstractRegistration {
public:
	CustomRegistration(AbstractRegistration& reg): AbstractRegistration(&reg)
		{ }
	virtual Processor *make(void) const { return 0; }
	virtual bool isFinal(void) const { return true; }
};

// Registration
MetaRegistration Processor::reg(
	"otawa::Processor",
	Version(1, 1, 0),
	p::base, 0,
	p::config, &Processor::OUTPUT,
	p::config, &Processor::LOG,
	p::config, &Processor::VERBOSE,
	p::config, &Processor::STATS,
	p::config, &Processor::TIMED,
	p::config, &Processor::LOG_LEVEL,
	p::end
);


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
 * @li @ref Processor::TIMED,
 * @li @ref Processor::LOG_LEVEL.
 *
 * @p Statistics
 * The statistics are recorded in the property list passed by @ref Processor::STATS.
 * @li @ref Processor::RUNTIME.
 *
 * @p Verbosity
 * OTAWA provides two way to activate verbosity in code processors.
 * @li passing the @ref Processor::VERBOSE to the processor property list
 * @li define the environment variable "OTAWA_VERBOSE"
 *
 * @ingroup proc
 */


/**
 * Build a simple anonymous processor.
 */
Processor::Processor(void): flags(0), stats(0), log_level(LOG_NONE), ws(0) {
	_reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	_reg->_base = &reg;
}


/**
 */
Processor::~Processor(void) {
	if(flags & IS_ALLOCATED)
		delete _reg;
}


/**
 * For internal use only.
 */
Processor::Processor(AbstractRegistration& registration)
: flags(0), stats(0), log_level(LOG_NONE), ws(0) {
	_reg = &registration;
}


/**
 * For internal use only.
 */
Processor::Processor(String name, Version version, AbstractRegistration& registration)
: flags(0), stats(0), log_level(LOG_NONE), ws(0) {
	_reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	_reg->_base = &registration;
	_reg->_name = name;
	_reg->_version = version;
}


/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param props		Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(elm::String name, elm::Version version,
const PropList& props): flags(0), stats(0), log_level(LOG_NONE) {
	_reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	_reg->_base = &reg;
	_reg->_name = name;
	_reg->_version = version;
	init(props);
}

/**
 * Build a new processor with name and version.
 * @param name		Processor name.
 * @param version	Processor version.
 * @deprecated
 */
Processor::Processor(String name, Version version)
: flags(0), stats(0), log_level(LOG_NONE), ws(0) {
	_reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	_reg->_base = &reg;
	_reg->_name = name;
	_reg->_version = version;
}


/**
 * Build a new processor.
 * @param			Configuration properties.
 * @deprecated		Configuration must be passed at the process() call.
 */
Processor::Processor(const PropList& props): flags(0), stats(0), log_level(LOG_NONE) {
	_reg = new NullRegistration();
	flags |= IS_ALLOCATED;
	_reg->_base = &reg;
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
	bool verbose;
	if(props.hasProp(VERBOSE))
		verbose = VERBOSE(props);
	else
		verbose = elm::system::System::hasEnv(VERBOSE_ENV);
	if(verbose) {
		flags |= IS_VERBOSE;
		log_level = LOG_BB;
	}
	else
		flags &= ~IS_VERBOSE;

	// Process timing
	if(logFor(LOG_PROC) || recordsStats()) {
		if(TIMED(props))
			flags |= IS_TIMED;
		else
			flags &= ~IS_TIMED;
	}

	// Progress
	_progress = PROGRESS(props);

	// configure statistics
	if(COLLECT_STATS(props))
		flags |= IS_COLLECTING;

	// get the log level
	log_level_t level = LOG_LEVEL(props);
	if(level)
		log_level = level;
}


/**
 * @fn Progress& Processor::progress(void);
 * Get the current progress handler.
 * @return	Progress handler.
 */


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

	// first, perform preparation
	if(!isPrepared())
		prepare(fw);

	// Perform configuration
	ws = fw;
	configure(props);

	// build the list of required features
	Vector<const AbstractFeature *> required;
	for(FeatureIter fuse(*_reg); fuse; fuse++)
		if(fuse->kind() == FeatureUsage::require && !required.contains(&fuse->feature()))
			required.add(&fuse->feature());

	// remove non-used invalidated features
	for(FeatureIter feature(*_reg); feature; feature++)
		if(feature->kind() == FeatureUsage::invalidate
		&& !_reg->uses(feature->feature())) {
			if(logFor(LOG_DEPS))
				log << "INVALIDATED: " << feature->feature().name()
					<< " by " << _reg->name() << io::endl;
			fw->invalidate(feature->feature());
			required.remove(&feature->feature());
		}

	// Get used feature
	for(FeatureIter feature(*_reg); !ws->isCancelled() && feature; feature++)
		if(feature->kind() == FeatureUsage::require
		|| feature->kind() == FeatureUsage::use) {
			if(logFor(LOG_DEPS)) {
				cstring kind = "USED";
				if(feature->kind() == FeatureUsage::require)
					kind = "REQUIRED";
				log << kind << ": " << feature->feature().name()
					<< " by " << _reg->name() << io::endl;
			}
			try {
				fw->require(feature->feature(), props);
			}
			catch(NoProcessorException& e) {
				log << "ERROR: no processor to implement " << feature->feature().name() << io::endl;
				throw UnavailableFeatureException(this, feature->feature());
			}
		}
	if(ws->isCancelled())
		return;

	// check before starting processor
	for(FeatureIter feature(*_reg); feature; feature++)
		if((feature->kind() == FeatureUsage::require
		|| feature->kind() == FeatureUsage::use)
		&& !fw->isProvided(feature->feature()))
			throw otawa::Exception(_ << "feature " << feature->feature().name()
				<< " is not provided after one cycle of requirements:\n"
				<< "stopping -- this may denotes circular dependencies.");

	// pre-processing actions
	if(logFor(LOG_CFG))
		log << "Starting " << name() << " (" << version() << ')' << io::endl;
	else if(logFor(LOG_PROC))
		log << "RUNNING: " << name() << " (" << version() << ')' << io::endl;
	elm::system::StopWatch swatch;
	if(isTimed())
		swatch.start();
	setup(fw);

	// Launch the work
	try {
		processWorkSpace(fw);
	}
	catch(ProcessorException& e) {
		cleanup(fw);
		throw e;
	}

	// Post-processing actions
	if(logFor(LOG_CFG))
		log << "Ending " << name();
	if(isTimed()) {
		swatch.stop();
		if(recordsStats())
			RUNTIME(*stats) = swatch.delay();
		if(logFor(LOG_CFG))
			log << " (" << ((double)swatch.delay() / 1000) << "ms)";
		else if(logFor(LOG_PROC))
			log << "INFO: time = " << ((double)swatch.delay() / 1000) << "ms";
	}
	if(logFor(LOG_CFG))
		log << io::endl;

	// cleanup used invalidated features
	for(FeatureIter feature(*_reg); feature; feature++)
		if(feature->kind() == FeatureUsage::invalidate
		&& _reg->uses(feature->feature())) {
			if(logFor(LOG_DEPS))
				log << "INVALIDATED: " << feature->feature().name()
					<< " by " << _reg->name() << io::endl;
			fw->invalidate(feature->feature());
			required.remove(&feature->feature());
		}

	// perform cleanup
	cleanup(fw);

	// add provided features
	for(FeatureIter feature(*_reg); feature; feature++)
		if(feature->kind() == FeatureUsage::provide) {
			if(logFor(LOG_DEPS))
				log << "PROVIDED: " << feature->feature().name()
					<< " by " << _reg->name() << io::endl;
			fw->provide(feature->feature(), &required);
		}

	// put the cleaners
	for(clean_list_t::Iterator clean(cleaners); clean; clean++) {
		FeatureDependency *dep = ws->getDependency((*clean).fst);
		ASSERTP(dep, "cleanup invoked for a not provided feature: " + (*clean).fst->name());
		dep->elm::CleanList::add((*clean).snd);
	}

	// record statistics
	if(isCollectingStats())
		collectStats(ws);
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
 * @param ws	Processed workspace.
 */
void Processor::setup(WorkSpace *ws) {
}


/**
 * This method is called after the end of the processor analysis to let it
 * do some clean up.
 * @param ws	Workspace to work on.
 */
void Processor::cleanup(WorkSpace *ws) {
}


/**
 * This method is invoked if the user has required to collect statistics
 * about the processor. In its default implementation, this method
 * does nothing but it may be customized if the current implementation
 * provides statistics. It is called after the main computation of the processor.
 *
 * Statistics must be objects inheriting from the class @ref StatCollector
 * and recorded with method recordStat().
 *
 * @param ws	Current workspace.
 */
void Processor::collectStats(WorkSpace *ws) {
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
	Processor::OUTPUT("otawa::Processor::OUTPUT", &io::out);


/**
 * This property identifier is used for setting the log stream used by
 * the processor to write messages (information, warning, error).
 */
Identifier<elm::io::OutStream *>
	Processor::LOG("otawa::Processor::LOG", &io::err);


/**
 * This property allows to activate collection of statistics for the
 * work of the current processor. @see StatInfo class.
 */
Identifier<bool>
	Processor::COLLECT_STATS("otawa::Processor::COLLECT_STATS", false);


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
 * Property passed in the configuration property list of a processor
 * to select the log level between LOG_PROC, LOG_CFG or LOG_BB.
 */
Identifier<Processor::log_level_t> Processor::LOG_LEVEL("otawa::Processor::LOG_LEVEL", LOG_NONE);


/**
 * Install
 */
Identifier<Progress *> Processor::PROGRESS("otawa::Processor::PROGRESS", &Progress::null);


/**
 * Usually called from a processor constructor, this method records a
 * required feature for the work of the current processor.
 * @param feature	Required feature.
 */
void Processor::require(const AbstractFeature& feature) {
	ASSERTP(!isPrepared(), "require() must only be called in constructor or in prepare()");
	if(!isAllocated()) {
		_reg = new CustomRegistration(*_reg);
		flags |= IS_ALLOCATED;
	}
	_reg->features.add(FeatureUsage(FeatureUsage::require, feature));
}


/**
 * Usually called from a processor constructor, this method records a feature
 * invalidated by the work of the current processor.
 * @param feature	Invalidated feature.
 */
void Processor::invalidate(const AbstractFeature& feature) {
	ASSERTP(!isPrepared(), "invalidate() must only be called in constructor or in prepare()");
	if(!isAllocated()) {
		_reg = new CustomRegistration(*_reg);
		flags |= IS_ALLOCATED;
	}
	_reg->features.add(FeatureUsage(FeatureUsage::invalidate, feature));
}


/**
 * Usually called from a processor constructor, this method records a feature
 * as used by the work of the current processor.
 * @param feature	Used feature.
 */
void Processor::use(const AbstractFeature& feature) {
	ASSERTP(!isPrepared(), "use() must only be called in constructor or in prepare()");
	if(!isAllocated()) {
		_reg = new CustomRegistration(*_reg);
		flags |= IS_ALLOCATED;
	}
	_reg->features.add(FeatureUsage(FeatureUsage::use, feature));
}


/**
 * Usually called from a processor constructor, this method records a feature
 * provided by the work of the current processor.
 * @param feature	Provided feature.
 */
void Processor::provide(const AbstractFeature& feature) {
	ASSERTP(!isPrepared(), "provide() must only be called in constructor or in prepare()");
	if(!isAllocated()) {
		_reg = new CustomRegistration(*_reg);
		flags |= IS_ALLOCATED;
	}
	_reg->features.add(FeatureUsage(FeatureUsage::provide, feature));
}


/**
 * @fn WorkSpace *Processor::workspace(void) const;
 * Get the current workspace.
 * @return	Current workspace.
 */


/**
 * This method called to let the processor customize its requirements
 * according to some generic feature. When overload, the original method
 * MUST be called at the end of the custom overriding implementation.
 * @param ws		Current workspace.
 */
void Processor::prepare(WorkSpace *ws) {
	flags |= IS_PREPARED;
}

/* statistics cleaner */
class StatCleaner: public Cleaner {
public:
	inline StatCleaner(WorkSpace *_ws, StatCollector *_collector): ws(_ws), collector(_collector) { }

	virtual void clean(void) {
		StatInfo::remove(ws, *collector);
		delete collector;
	}

private:
	WorkSpace *ws;
	StatCollector *collector;
};

/**
 * Add a statistics collector to the current workspace collection.
 * The statistics object is tracked and linked to the feature.
 * In case of the feature invalidation, the statistics information
 * instance is purged of the statistics collector.
 * @param feature		Feature to link the collector to.
 * @param collector		Statistics collector to add.
 */
void Processor::recordStat(const AbstractFeature& feature, StatCollector *collector) {
	ASSERTP(isCollectingStats(), "no statistics collection at this time");
	StatInfo::add(workspace(), *collector);
	addCleaner(feature, new StatCleaner(workspace(), collector));
}


/**
 * @fn void Processor::addCleaner(const AbstractFeature& feature, Cleaner *cleaner);
 * Add a cleaner for the given feature. A cleaner is an object with a virtual destructor
 * that is called when the associated feature is removed: this let the processor to release
 * and clean up the resources it allocated to implement the provided features.
 * @param feature	Feature the cleaner apply to.
 * @param cleaner	Cleaner to add.
 */


/**
 * @fn T *Processor::track(AbstractFeature& feature, T *object);
 * Track the release of an allocated object with the given feature.
 * Ensure that the given object will be deleted when the feature is invalidated.
 * It may be used as below:
 * @code
 *		MyClass *object = track(MY_FEATURE, new MyClass());
 * @endcode
 * @param feature	Feature the cleaner applies to.
 * @param object	Object to delete.
 */


/**
 * @fn void Processor::track(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref);
 * Track the release of an allocated object assigned to an identifier relatively to the
 * given feature. When the feature is deleted, the object is fried and the identifier
 * is removed. It is used as below:
 * @code
 * 		track(MY_FEATURE, MY_ID(props) = value);
 * @endcode
 *
 * @param feature	Linked feature.
 * @param ref		Reference to the identifier to remove.
 */


/**
 * @fn void Processor::track(const AbstractFeature& feature, const Ref<T *, const Identifier<T *> >& ref);
 * Track the release of an allocated object assigned to an identifier relatively to the
 * given feature. When the feature is deleted, the object is fried and the identifier
 * is removed. It is used as below:
 * @code
 * 		track(MY_FEATURE, MY_ID(props) = value);
 * @endcode
 *
 * @param feature	Linked feature.
 * @param ref		Reference to the identifier to remove.
 */


/**
 * @fn bool Processor::logFor(log_level_t tested) const;
 * Test if the logging for the given logel is activated.
 * @param tested	Tested log level.
 * @return			True if the log level is activated, false else.
 */


/**
 * @fn log_level_t Processor::logLevel(void) const;
 * Get the current log level.
 * @return	Current log level.
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


/**
 * @class Progress
 * This class is an interface to get progress information about
 * execution of a processor. To use it, you have to overload its pure
 * virtual members and to pass it to the processor configuration
 * via @ref otawa::Processor::PROGRESS property.
 *
 * @warning Not all processor supports progress and some of them
 * may ignore it.
 * @ingroup proc
 */


// Null progress implementation
class NullProgress: public Progress {
public:
	virtual ~NullProgress(void) { }
	virtual void start(const Processor& processor, mode_t mode, int max = 0) { }
	virtual void stop(void) { }
	virtual void report(string info, int level) { }
	virtual void restart(string info)  { }
};
static NullProgress null_progress;


/**
 * Progress handler that do nothing.
 */
Progress& Progress::null = null_progress;


/**
 * @fn void Progress::start(const Processor& processor, mode_t mode, int max);
 * This method is called when the processor is starting. The mode parameter
 * describe the way the processor is working.
 *
 * A mode @ref otawa::Progess::living can not produce statistics about progress.
 * The report() is called regularly to inform that the processor is living.
 * The max parameter is meaningless.
 *
 * A mode @ref otawa::Progress::linear says the report() method is called
 * each a progress is performed until the end. max represents the number
 * of stages before end.
 *
 * A mode @ref otawa::Progress::fixpoint informs the computation produces
 * an undefined number of steps but each step is separated by a call to
 * restart(). max represents the number of level before end.
 *
 * @param processor		Current processor.
 * @param mode			Current mode.
 * @param max			Number of levels.
 */


/**
 * @fn void Progress::stop(void);
 * Called at the end of the work.
 */


/**
 * @fn void Progress::report(string info, int level);
 * Called each time to inform about the progression.
 * @param info	Info about the report.
 * @param level	Current level.
 */


/**
 * @fn void Progress::restart(string info);
 * This method is only called in @ref otawa::Processor::fixpoint mode
 * at each step start.
 * @param info	Current step information.
 */


} // otawa
