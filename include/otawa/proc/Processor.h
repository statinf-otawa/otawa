/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/proc/Processor.h -- Processor class interface.
 */
#ifndef OTAWA_PROC_PROCESSOR_H
#define OTAWA_PROC_PROCESSOR_H

#include <elm/io.h>
#include <elm/util/Version.h>
#include <elm/system/StopWatch.h>
#include <otawa/prog/FrameWork.h>

namespace otawa {

// Processor class
class Processor {
	elm::String _name;
	elm::Version _version;
	static const unsigned long TIMED = 0x01;
	static const unsigned long VERBOSE = 0x02;
	unsigned long flags;
	void init(const PropList& props);
protected:
	elm::io::Output out;
	PropList *stats;
	inline bool isVerbose(void) const;
	inline bool isTimed(void) const;
	inline bool recordsStats(void) const;
public:
	//static Identifier ID_Output;
	
	Processor(const PropList& props = PropList::EMPTY);
	Processor(elm::String name, elm::Version version = elm::Version::ZERO,
		const PropList& props = PropList::EMPTY);
	inline elm::String name(void) const;
	inline elm::Version version(void) const;

	virtual void configure(const PropList& props);
	void process(FrameWork *fw);
	virtual void processFrameWork(FrameWork *fw) = 0;
};


// Configuration Properties
extern GenericIdentifier<elm::io::OutStream *> PROC_OUTPUT;
extern GenericIdentifier<PropList *> PROC_STATS;
extern GenericIdentifier<bool> PROC_TIMED;
extern GenericIdentifier<bool> PROC_VERBOSE;


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
