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
#ifndef OTAWA_PROC_PROCESSOR_H
#define OTAWA_PROC_PROCESSOR_H

#include <elm/io.h>
#include <elm/util/Version.h>
#include <elm/system/StopWatch.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
//#include <otawa/prog/WorkSpace.h>
#include <otawa/properties.h>
#include <otawa/proc/ProcessorException.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;
class AbstractFeature;
class Registration;
class Configuration;
class WorkSpace;
class FeatureDependency;

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
	elm::io::Output log;
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
	static Identifier<elm::io::OutStream *> LOG;
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


// NoProcessorException class
class NoProcessorException: public Exception {
};


// UnavailableFeatureException class
class UnavailableFeatureException: public ProcessorException {
public:
	
 	inline UnavailableFeatureException(
 		const Processor *processor,
 		const AbstractFeature& feature
 	): ProcessorException(*processor, ""), f(feature) { }
 		
 	inline const AbstractFeature& feature(void) const { return f; }
 	virtual String 	message(void); 
private:
	const AbstractFeature& f;
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
	return flags & IS_TIMED;
}

inline bool Processor::recordsStats(void) const {
	return stats;
}

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
