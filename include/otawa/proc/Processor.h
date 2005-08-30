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
#include <otawa/prog/FrameWork.h>

namespace otawa {

// Processor class
class Processor {
	elm::String _name;
	elm::Version _version;
	void init(const PropList& props);
protected:
	elm::io::Output out;
public:
	static Identifier ID_Output;
	
	Processor(const PropList& props = PropList::EMPTY);
	Processor(elm::String name, elm::Version version = elm::Version::ZERO,
		const PropList& props = PropList::EMPTY);
	inline elm::String name(void) const;
	inline elm::Version version(void) const;

	virtual void configure(const PropList& props);
	virtual void processFrameWork(FrameWork *fw) = 0;
};

// Inlines
inline elm::String Processor::name(void) const {
	return _name;
}

inline elm::Version Processor::version(void) const {
	return _version;
}

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
