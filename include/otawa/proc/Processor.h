/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	Processor class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_H
#define OTAWA_PROC_PROCESSOR_H

#include <elm/io.h>
#include <elm/util/Version.h>
#include <elm/system/StopWatch.h>
#include <otawa/prog/FrameWork.h>
#include <otawa/proc/ProcessorException.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;
class AbstractFeature;

// Processor class
class Processor {
	elm::String _name;
	elm::Version _version;
	void init(const PropList& props);
	Vector<const AbstractFeature *> required;
	Vector<const AbstractFeature *> provided;

protected:
	static const unsigned long TIMED = 0x01;
	static const unsigned long VERBOSE = 0x02;
	unsigned long flags;
	elm::io::Output out;
	PropList *stats;
	
	// Facility methods
	inline bool isVerbose(void) const;
	inline bool isTimed(void) const;
	inline bool recordsStats(void) const;
	void require(const AbstractFeature& feature);
	void provide(const AbstractFeature& feature);
	void warn(CString format, VarArg args);
	void warn(CString format, ...);

	// Overwritable methods
	virtual void processFrameWork(FrameWork *fw) = 0;
	virtual void setup(FrameWork *fw);
	virtual void cleanup(FrameWork *fw);

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
	void process(FrameWork *fw, const PropList& props = PropList::EMPTY);
};


// NullProcessor class
class NullProcessor: public Processor {
public:
	NullProcessor(void);
};


// NoProcessor class
class NoProcessor: public Processor {
protected:
	virtual void processFrameWork(FrameWork *fw);
public:
	NoProcessor(void);
}; 


// Configuration Properties
extern Identifier<elm::io::OutStream *> PROC_OUTPUT;
extern Identifier<PropList *> PROC_STATS;
extern Identifier<bool> PROC_TIMED;
extern Identifier<bool> PROC_VERBOSE;
extern Identifier<bool> RECURSIVE;

// Statistics Properties
extern Identifier<elm::system::time_t> PROC_RUNTIME;

// Inlines
inline elm::String Processor::name(void) const {
	return _name;
}

inline elm::Version Processor::version(void) const {
	return _version;
}

inline bool Processor::isVerbose(void) const {
	return flags & VERBOSE;
}

inline bool Processor::isTimed(void) const {
	return stats && (flags & TIMED);
}

inline bool Processor::recordsStats(void) const {
	return stats;
}

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
