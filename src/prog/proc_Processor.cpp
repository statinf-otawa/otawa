/*
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

#include <elm/io/BufferedOutStream.h>
#include <elm/sys/System.h>
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
 * @li methods providing meaningful results in the object of the program representation.
 *
 * The feature are an easier way to inform the framework that some services are provided
 * and to group properties and methods in logical sets. A feature is mainly implemented
 * by an @ref otawa::AbstractFeature.
 *
 * It may be implemented using the command otawa::p::feature with a default code processor
 * (see below) to satisfy the service if it is not provided. As the goal of feature is
 * to share some data between analyzes, it is first declared in a header file, for
 * example "my_feature.h" :
 * @code
 * #include <otawa/proc/Feature.h>
 * #include "MyDefaultProcessor.h"
 *
 * extern p::feature MY_FEATURE;
 * @endcode
 *
 * Then, it must defined in a source file, for example, "my_feature.cpp",
 * with a default builder, @c MyProcessor in this example:
 * @code
 * #include "my_feature.h"
 *
 * p::feature MY_FEATURE("MY_FEATURE", p::make<MyProcessor>());
 * @endcode
 *
 * Note that a feature must have a unique name: duplicate names in features names will be
 * detected at system startup and will cause abortion of the application. Although it is
 * not mandatory, it is advised (1) to give a name in upper case with underscores to separate
 * names and (2) to give the full-qualified C++ name. This convention will help to
 * retrieve features when dynamic module loading is used.
 *
 * Some features can come with a specific interface providing some special facilities
 * that cannot be expressed using properties. This interface is typically an abstract class.
 * To declare such a feature, one has to use "p::interfaced_feature" as:
 * @code
 * p::interfaced_feature<MyInterface> MY_FEATURE;
 * @endcode
 *
 * To obtain this interface in a workspace wher the feature is provided, one has to
 * use feature function, AbstractFeature::get() as below:
 * @code
 * WorkSpace *ws;
 * ws->require(MY_FEATURE);
 * MyInterface *inter = MY_FEATURE.get(ws);
 * @endcode
 *
 * To implement, any code processor implementing the feature has to provide a function
 * named @ref Processor::interfaceFor() and to return the corresponding interface:
 * @code
 * class MyProcessor: public Processor {
 * public:
 * 	...
 * 	void *interfaceFor(const AbstractFeature& f) override { return inter; }
 * 	...
 * private:
 * 	...
 * 	MyInterface *inter;
 * };
 * @endcode
 *
 *
 * @par Code Processors
 *
 * A code processor performs an analysis, that is, scans the program representation,
 * extracts some properties and provide them to the remaining analyzes. To organize the
 * properties, they are grouped logically in features. To organize this work, the
 * code processors records itself to OTAWA in order to:
 * @li ensure that the required features are available,
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
 * This class provides also a common way to invoke analyzes. The configuration
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
 * WorkSpace *ws;
 * ws->run<MyProcessor>(props);
 * @endcode
 *
 * For example, to redirect the log output of the processor and to activate
 * verbosity, and to compute the domination in a CFG, we may do:
 * @code
 * PropList props;
 * Processor::VERBOSE(props) = true;
 * Processor::LOG(props) = System::newFile("my_log.txt");
 * ws->run<Domination>(props);
 * @endcode
 *
 * Notice that the configuration properties are passed to all processor invoked to provide
 * features required by the invoked processor.
 *
 * Code processors are usually invoked directly by a user or indirectly to implement a
 * specific feature. Whatever the case, the code processor follows this calling cycle:
 *	* @ref Processor::prepare() -- called before satisfying its required/used features,
 *	* @ref Processor::setup() -- to set up some common resources
 *	* @ref Processor::processWorkSpace() -- to perform the analysis itself
 *	* @ref Processor::cleanup() -- to clean the resources allocated in setup()
 *	* @ref Processor::commit() -- to install properties required its provided features,
 *	* @ref Processor::destroy() -- called at deletion/invalidation of its provided features to clean up properties installed in commit().
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

/**
 * A registration to customize.
 */
class CustomRegistration: public AbstractRegistration {
public:
	CustomRegistration(AbstractRegistration& reg): AbstractRegistration(&reg)
		{ }
	virtual Processor *make(void) const { return 0; }
};

static AbstractRegistration& trackit(AbstractRegistration& reg) {
	return reg;
}

// null registration
class NullRegistration: public AbstractRegistration {
public:
	NullRegistration(void): AbstractRegistration(this) { }
	virtual Processor *make(void) const { return 0; }
	virtual bool isNull(void) const { return true; }
};


/**
 * Null abstract registration.
 */
AbstractRegistration& AbstractRegistration::null = single<NullRegistration>();



// Registration
p::declare Processor::reg = p::init("otawa::Processor", Version(1, 2, 0))
	.base(trackit(AbstractRegistration::null))
	.config(OUTPUT)
	.config(LOG)
	.config(VERBOSE)
	.config(STATS)
	.config(TIMED)
	.config(LOG_LEVEL)
	.config(LOG_FOR);


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
Processor::Processor(void)
: stats(nullptr), ws(nullptr), _progress(nullptr), _dump(nullptr) {
	_reg = new CustomRegistration(reg);
	flags |= IS_ALLOCATED;
}


/**
 */
Processor::~Processor(void) {
	if(flags & IS_TIED)
		for(FeatureIter f(*_reg); f(); f++)
			if(f->kind() == FeatureUsage::provide) {
				ws->invalidate(f->feature());
				break;
			}
	if(flags & IS_ALLOCATED)
		delete _reg;
}


/**
 * For internal use only.
 */
Processor::Processor(AbstractRegistration& registration)
: stats(nullptr), ws(nullptr), _progress(nullptr), _dump(nullptr) {
	_reg = &registration;
}


/**
 * For internal use only.
 */
Processor::Processor(String name, Version version, AbstractRegistration& registration)
: stats(nullptr), ws(nullptr), _progress(nullptr), _dump(nullptr) {
	_reg = new CustomRegistration(reg);
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
const PropList& props): stats(nullptr), _dump(nullptr) {
	_reg = new CustomRegistration(reg);
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
: stats(nullptr), ws(nullptr), _progress(nullptr), _dump(nullptr) {
	_reg = new CustomRegistration(reg);
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
Processor::Processor(const PropList& props): stats(0) {
	_reg = new CustomRegistration(reg);
	flags |= IS_ALLOCATED;
	_reg->_base = &reg;
	init(props);
}


/**
 */
void Processor::init(const PropList& props) {

	// process output
	out.setStream(*OUTPUT(props));
	log.setStream(*LOG(props));

	// process statistics
	stats = STATS(props);

	// process timing
	if(logFor(LOG_PROC) || recordsStats()) {
		if(TIMED(props))
			flags |= IS_TIMED;
		else
			flags &= ~IS_TIMED;
	}

	// process dumping
	if(DUMP(props))
		flags |= IS_DUMPING;
	else
		for(auto f: DUMP_FOR.all(props))
			if(f == name())
				flags |= IS_DUMPING;
	if(isDumping()) {
		sys::Path p = DUMP_TO(props);
		if(!p)
			_dump = &out.stream();
		else {
			try {
				auto s = p.write();
				_dump = new io::BufferedOutStream(s, true);
			}
			catch(io::IOException& e) {
				throw ProcessorException(*this, _ << "cannot dump to " << p << ": " << e.message());
			}
		}
	}

	// progress
	_progress = PROGRESS(props);

	// configure statistics
	if(COLLECT_STATS(props))
		flags |= IS_COLLECTING;
}


/**
 * @fn Progress& Processor::progress(void);
 * Get the current progress handler.
 * @return	Progress handler.
 */


/**
 * Process the given framework.
 * @param fw	Framework to process.
 * @deprecated	Use @ref processWorkSpace() instead.
 */
void Processor::processWorkSpace(WorkSpace *fw) {
}


/**
 * This method may be called for configuring a processor thanks to information
 * passed in the property list.
 * @param props	Configuration information.
 */
void Processor::configure(const PropList& props) {
	init(props);
	Monitor::configure(props, name());
}


/**
 * This function runs the analysis on the given workspace.
 * @param ws	Work space to run on.
 */
void Processor::run(WorkSpace *ws) {
	this->ws = ws;
	setWorkspace(ws);

	// pre-processing actions
	if(!isQuiet()) {
		if(logFor(LOG_CFG))
			log << "Starting " << name() << " (" << version() << ')' << io::endl;
		else if(logFor(LOG_PROC))
			log << "RUNNING: " << name() << " (" << version() << ')' << io::endl;
	}

	// record time
	elm::sys::StopWatch swatch;
	if(isTimed())
		swatch.start();

	// Launch the work
	setup(ws);
	try {
		processWorkSpace(ws);
	}
	catch(ProcessorException& e) {
		cleanup(ws);
		throw e;
	}
	cleanup(ws);

	// Post-processing actions
	if(!isQuiet() && logFor(LOG_CFG))
		log << "Ending " << name();
	if(isTimed()) {
		swatch.stop();
		if(recordsStats())
			RUNTIME(*stats) = swatch.delay().micros();
		if(!isQuiet()) {
			if(logFor(LOG_CFG))
				log << " (" << (swatch.delay().micros() / 1000.) << "ms)";
			else if(!isQuiet() && logFor(LOG_PROC))
				log << "INFO: time = " << (swatch.delay().micros() / 1000) << "ms";
		}
	}
	if(!isQuiet() && logFor(LOG_CFG))
		log << io::endl << io::endl;

	// dump if required
	if(isDumping()) {
		Output out(*_dump);
		dump(ws, out);
		if((flags & CLOSE_DUMP) != 0) {
			flags &= ~CLOSE_DUMP;
			delete _dump;
			_dump = nullptr;
		}
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
 * do some clean up. Some data such as Identifiers set in in the processor
 * analysis may still be used in the later analysis. Please refer to
 * elm::addCleaner() to clean the such data when the feature provided by
 * the processor analysis is removed/invalidated.
 * @param ws	Workspace to work on.
 */
void Processor::cleanup(WorkSpace *ws) {
}

/**
 * This processor is called as soon as all the required and invalidated
 * features has been removed to let the processor install its own
 * properties. This is specially useful when the current processor
 * uses/requires a feature that it is also providing.
 *
 * The default implementation does nothing.
 *
 * @param ws	Current workspace.
 */
void Processor::commit(WorkSpace *ws) {
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
 * This method is called when the properties produced by a processor
 * are no more useful and can be release. This happens (a) when the workspace
 * is release or (b) the features provided by the processor are invalidated.
 *
 * The processor has to retain the properties of the provided features until
 * this function is called.
 *
 * The default implementation activates the clean list.
 */
void Processor::destroy(WorkSpace *ws) {
	for(List<Cleaner *>::Iter clean = *cleaners; clean(); clean++)
		delete *clean;
	cleaners.clear();
}


/**
 * Function called for debugging purpose when dump properties (otawa::DUMP,
 * otawa::DUMP_FOR) are provided. The default implementation does nothing.
 * The dumped data should be human readable.
 * @param ws	Current workspace.
 * @param out	Output to dump to.
 */
void Processor::dump(WorkSpace *ws, Output& out) {

}


/**
 * Method called after registration requirements
 * to support dynamic requirements. As a default, do nothing.
 * @param ws	Current workspace.
 * @param props	Current properties.
 */
void Processor::requireDyn(WorkSpace *ws, const PropList& props) {

}


/**
 * Display a warning.
 * @param message	Message to display.
 */
void Processor::warn(const String& message) {
	log << "WARNING: " << name() << ' ' << version()
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
 * @deprecated
 */
p::id<elm::io::OutStream *>& Processor::OUTPUT = otawa::OUTPUT;


/**
 * This property identifier is used for setting the log stream used by
 * the processor to write messages (information, warning, error).
 * @deprecated
 */
p::id<elm::io::OutStream *>& Processor::LOG = otawa::LOG;


/**
 * This property allows to activate collection of statistics for the
 * work of the current processor. @see StatInfo class.
 */
p::id<bool>
	Processor::COLLECT_STATS("otawa::Processor::COLLECT_STATS", false);


/**
 * This property identifiers is used to pass a property list to the processor
 * that will be used to store statistics about the performed work. Implicitly,
 * passing such a property activates the statistics recording facilities.
 */
p::id<PropList *> Processor::STATS("otawa::Processor::STATS", 0);


/**
 * If the value of the associated property is true (default to false), time
 * statistics will also be collected with other processor statistics. Passing
 * such a property without @ref PROC_STATS has no effects.
 */
p::id<bool> Processor::TIMED("otawa::Processor::TIMED", false);


/**
 * This property identifier is used to store in the statistics of a processor
 * the overall run time of the processor work.
 */
p::id<elm::sys::time_t> Processor::RUNTIME("otawa::Processor::RUNTIME", 0);


/**
 * This property activates the verbose mode of the processor: information about
 * the processor work will be displayed.
 */
p::id<bool>& Processor::VERBOSE = otawa::VERBOSE;


/**
 * Property passed in the configuration property list of a processor
 * to select the log level between LOG_PROC, LOG_CFG or LOG_BB.
 */
p::id<Processor::log_level_t>& Processor::LOG_LEVEL = otawa::LOG_LEVEL;



/**
 * Logging message will only be displayed for the named processor.
 * If no LOG_PROC is available in the configuration, all code processors are logged.
 */
p::id<string>& Processor::LOG_FOR = otawa::LOG_FOR;


/**
 * Record a progress listener for the execution of the processors.
 */
p::id<Progress *> Processor::PROGRESS("otawa::Processor::PROGRESS", &Progress::null);


/**
 * Dump the results of all analyzes.
 * @ingroup proc
 */
p::id<bool> DUMP("otawa::DUMP", false);


/**
 * Dump the result of the analysis which name is passed as argument.
 * Several instances of this property can be given.
 * @ingroup proc
 */
p::id<string> DUMP_FOR("otawa::DUMP_FOR", "");


/**
 * Dump the result to the given file path.
 * @ingroup proc
 */
p::id<string> DUMP_TO("otawa::DUMP_TO", "");


/**
 * Dump the results to the given stream.
 * @ingroup proc
 */
p::id<elm::io::OutStream *> DUMP_STREAM("otawa::DUMP_STREAM", nullptr);


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
	_reg->_feats.add(FeatureUsage(FeatureUsage::require, feature));
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
	_reg->_feats.add(FeatureUsage(FeatureUsage::invalidate, feature));
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
	_reg->_feats.add(FeatureUsage(FeatureUsage::use, feature));
}


/**
 * Get the interface corresponding to the given feature. The feature
 * can only be part of the processor provided feature. This function
 * can only be called once the processor has been executed.
 *
 * @param feature	Feature of the looked interface.
 * @return			Null pointer or interface corresponding to the feature.
 */
void *Processor::interfaceFor(const AbstractFeature& feature) {
	throw ProcessorException(*this, _ << "interface for " << feature.name() << " not implemented!");
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
	_reg->_feats.add(FeatureUsage(FeatureUsage::provide, feature));
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
 * @deprecated
 */
void Processor::recordStat(const AbstractFeature& feature, StatCollector *collector) {
	ASSERTP(isCollectingStats(), "no statistics collection at this time");
	StatInfo::add(workspace(), *collector);
	addCleaner(feature, new StatCleaner(workspace(), collector));
}

/**
 * Add a statistics collector to the current workspace collection.
 * The statistics object is tracked by the processor and will
 * be fried automatically when needed.
 * @param collector		Statistics collector to add.
 */
void Processor::record(StatCollector *collector) {
	ASSERTP(isCollectingStats(), "no statistics collection at this time");
	StatInfo::add(workspace(), *collector);
	track(new StatCleaner(workspace(), collector));
}


/**
 * Track the given cleaner, that is, record the cleaner in the analysis
 * and trigger when the analysis is deleted.
 * @param cleaner	Cleaner to add.
 */
void Processor::track(Cleaner *cleaner) {
	cleaners.add(cleaner);
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


static NullProcessor _null;
/**
 * Singleton representing a null processor.
 */
Processor& Processor::null = _null;

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
 * @class NoProcessor
 * A processor which execution causes an exception throw. Useful for features
 * without default definition.
 * @ingroup proc
 */

///
void NoProcessor::processWorkSpace(WorkSpace *fw) {
	throw NoProcessorException();
}


///
NoProcessor::NoProcessor():
	Processor("otawa::NoProcessor", Version(1, 0, 0))
{
}

/**
 * @class NoProcessorException
 * This exception is raised by the NoProcessor class when it is called
 * to implement a feature. The NoProcessor is used to denote features
 * without default processor.
 * @ingroup proc
 */
 
///
String NoProcessorException::message() {
	return "no processor defined for this feature";
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
