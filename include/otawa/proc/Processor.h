/*
 *	$Id$
 *	Processor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-7, IRIT UPS <casse@irit.fr>
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
#include <elm/util/Cleaner.h>
#include <elm/util/Version.h>
#include <elm/system/StopWatch.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/HashTable.h>
#include <elm/util/Cleaner.h>
#include <otawa/properties.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/proc/Registration.h>

namespace otawa {

using namespace elm;
using namespace elm::genstruct;
class AbstractFeature;
class Configuration;
class WorkSpace;
class FeatureDependency;


// Processor class
class Processor {

	template <class T>
	class Deletor: public elm::Cleaner {
	public:
		inline Deletor(const Ref<T *, Identifier<T *> >& ref): _ref(ref) { }
		virtual ~Deletor(void) { delete _ref.get(); _ref.remove(); }
	private:
		Ref<T *, Identifier<T *> > _ref;
	};

public:
	static struct __init: NullRegistration { __init(void); } __reg;
	static void init(void);

	// Constructors
	Processor(void);
	Processor(AbstractRegistration& registration);
	Processor(String name, Version version);
	Processor(String name, Version version, AbstractRegistration& registration);
	virtual ~Processor(void);

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

	// Deprecated
	Processor(const PropList& props);
	Processor(elm::String name, elm::Version version, const PropList& props);

protected:
	static const unsigned long IS_TIMED = 0x01;
	static const unsigned long IS_VERBOSE = 0x02;
	static const unsigned long IS_ALLOCATED = 0x04;
	unsigned long flags;
	elm::io::Output out;
	elm::io::Output log;
	PropList *stats;

	// Facility methods
	friend class FeatureRequirer;
	inline bool isVerbose(void) const;
	inline bool isTimed(void) const;
	inline bool recordsStats(void) const;
	void require(const AbstractFeature& feature);
	void provide(const AbstractFeature& feature);
	void invalidate(const AbstractFeature& feature);
	void use(const AbstractFeature& feature);
	void warn(const String& message);
	inline WorkSpace *workspace(void) const { return ws; }
	inline void addCleaner(const AbstractFeature& feature, Cleaner *cleaner)
		{ cleaners.add(clean_t(&feature, cleaner)); }
	template <class T> void track(const AbstractFeature& feature, T *object)
		{ addCleaner(feature, new Deletor<T>(object)); return object; }
	template <class T> void track(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }

	// Overwritable methods
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void setup(WorkSpace *fw);
	virtual void cleanup(WorkSpace *fw);

	// Deprecated
	virtual void processFrameWork(WorkSpace *fw);

private:
	void init(const PropList& props);
	AbstractRegistration *reg;
	WorkSpace *ws;
	typedef Pair<const AbstractFeature *, Cleaner *> clean_t;
	typedef elm::genstruct::SLList<clean_t> clean_list_t;
	clean_list_t cleaners;
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
	return reg->name();
}

inline elm::Version Processor::version(void) const {
	return reg->version();
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
