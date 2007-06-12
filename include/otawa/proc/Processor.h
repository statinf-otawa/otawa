/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	Processor class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_H
#define OTAWA_PROC_PROCESSOR_H

#include <elm/io.h>
#include <elm/util/Version.h>
#include <elm/system/StopWatch.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/ProcessorException.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;
class AbstractFeature;
class Registration;
class Configuration;

// Processor class
class Processor {
	friend class Registration;
	elm::String _name;
	elm::Version _version;
	void init(const PropList& props);
	Vector<const AbstractFeature *> required;
	Vector<const AbstractFeature *> provided;
	Vector<const AbstractFeature *> invalidated;
	Vector<Configuration *> configs;

protected:
	static const unsigned long IS_TIMED = 0x01;
	static const unsigned long IS_VERBOSE = 0x02;
	unsigned long flags;
	elm::io::Output out;
	PropList *stats;
	
	// Facility methods
	inline bool isVerbose(void) const;
	inline bool isTimed(void) const;
	inline bool recordsStats(void) const;
	void require(const AbstractFeature& feature);
	void provide(const AbstractFeature& feature);
	void invalidate(const AbstractFeature& feature);
	inline void config(Configuration& config) { configs.add(&config); }
	void warn(const String& message);

	// Overwritable methods
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);
	virtual void cleanup(WorkSpace *fw);

	// Deprecared
	virtual void processFrameWork(WorkSpace *fw);

public:

	// Constructors
	Processor(const PropList& props = PropList::EMPTY);
	Processor(elm::String name, elm::Version version, const PropList& props);
	Processor(String name, Version version);
	
	// Accessors
	inline elm::String name(void) const;
	inline elm::Version version(void) const;

	// Mutators
	virtual void configure(const PropList& props);
	void process(WorkSpace *fw, const PropList& props = PropList::EMPTY);

	// Configuration Properties
	static Identifier<elm::io::OutStream *> OUTPUT;
	static Identifier<PropList *> STATS;
	static Identifier<bool> TIMED;
	static Identifier<bool> VERBOSE;
	static Identifier<bool> RECURSIVE;

	// Statistics Properties
	static Identifier<elm::system::time_t> RUNTIME;
};


// NullProcessor class
class NullProcessor: public Processor {
public:
	NullProcessor(void);
};


// NoProcessor class
class NoProcessor: public Processor {
protected:
	virtual void processWorkSpace(WorkSpace *fw);
public:
	NoProcessor(void);
}; 


// Inlines
inline elm::String Processor::name(void) const {
	return _name;
}

inline elm::Version Processor::version(void) const {
	return _version;
}

inline bool Processor::isVerbose(void) const {
	return flags & IS_VERBOSE;
}

inline bool Processor::isTimed(void) const {
	return stats && (flags & IS_TIMED);
}

inline bool Processor::recordsStats(void) const {
	return stats;
}

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
