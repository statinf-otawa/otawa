/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/proc/Processor.h -- Processor class interface.
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
	inline bool isVerbose(void) const;
	inline bool isTimed(void) const;
	inline bool recordsStats(void) const;
	virtual void processFrameWork(FrameWork *fw) = 0;
	void require(const AbstractFeature& feature);
	void provide(const AbstractFeature& feature);

public:	
	Processor(const PropList& props = PropList::EMPTY);
	Processor(elm::String name, elm::Version version, const PropList& props);
	Processor(String name, Version version);
	inline elm::String name(void) const;
	inline elm::Version version(void) const;

	virtual void configure(const PropList& props);
	void process(FrameWork *fw, const PropList& props = PropList::EMPTY);
};

// Configuration Properties
extern GenericIdentifier<elm::io::OutStream *> PROC_OUTPUT;
extern GenericIdentifier<PropList *> PROC_STATS;
extern GenericIdentifier<bool> PROC_TIMED;
extern GenericIdentifier<bool> PROC_VERBOSE;
extern GenericIdentifier<bool> RECURSIVE;

// Statistics Properties
extern GenericIdentifier<elm::system::time_t> PROC_RUNTIME;

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
