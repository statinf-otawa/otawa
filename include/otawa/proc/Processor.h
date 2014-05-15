/*
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
class Progress;
class StatCollector;


// Processor class
class Processor {

	template <class T>
	class Remover: public elm::Cleaner {
	public:
		inline Remover(const Ref<T, Identifier<T> >& ref): _ref(ref.props(), ref.id()) { }
		virtual void clean(void) { _ref.remove(); }
	protected:
		inline const Ref<T, Identifier<T> >& ref(void) const { return _ref; }
		Ref<T, Identifier<T> > _ref;
	};

	template <class T>
	class Deletor: public Remover<T *> {
	public:
		inline Deletor(const Ref<T *, Identifier<T *> >& ref): Remover<T *>(ref) { }
		virtual void clean(void) { delete Remover<T *>::ref().get(); Remover<T *>::clean(); }
	};

public:

	typedef enum log_level_t {
		LOG_NONE = 0,
		LOG_PROC = 1,
		LOG_FILE = 2,
		LOG_DEPS = LOG_FILE,
		LOG_FUN = 3,
		LOG_CFG = LOG_FUN,
		LOG_BLOCK = 4,
		LOG_BB = LOG_BLOCK,
		LOG_INST = 5
	} log_level_t;

	// Constructors
	Processor(void);
	Processor(AbstractRegistration& registration);
	Processor(String name, Version version);
	Processor(String name, Version version, AbstractRegistration& registration);
	virtual ~Processor(void);
	static MetaRegistration reg;

	// Accessors
	inline elm::String name(void) const { return _reg->name(); }
	inline elm::Version version(void) const { return _reg->version(); }

	// Mutators
	virtual void configure(const PropList& props);
	void process(WorkSpace *ws, const PropList& props = PropList::EMPTY);

	// Configuration Properties
	static Identifier<elm::io::OutStream *> OUTPUT;
	static Identifier<elm::io::OutStream *> LOG;
	static Identifier<PropList *> STATS;
	static Identifier<bool> TIMED;
	static Identifier<bool> VERBOSE;
	static Identifier<bool> RECURSIVE;
	static Identifier<Progress *> PROGRESS;
	static Identifier<bool> COLLECT_STATS;
	static Identifier<log_level_t> LOG_LEVEL;

	// Statistics Properties
	static Identifier<elm::system::time_t> RUNTIME;

	// Deprecated
	Processor(const PropList& props);
	Processor(elm::String name, elm::Version version, const PropList& props);

protected:
	static const t::uint32
		IS_TIMED		= 0x01,
		IS_VERBOSE		= 0x02,
		IS_ALLOCATED	= 0x04,
		IS_PREPARED		= 0x08,
		IS_COLLECTING	= 0x10;
	unsigned long flags;
	elm::io::Output out;
	elm::io::Output log;
	PropList *stats;

	// accessors
	friend class FeatureRequirer;
	inline bool isVerbose(void) const { return flags & IS_VERBOSE; }
	inline bool isTimed(void) const { return flags & IS_TIMED; }
	inline bool recordsStats(void) const { return stats; }
	inline bool isAllocated(void) const { return flags & IS_ALLOCATED; }
	inline bool isPrepared(void) const { return flags & IS_PREPARED; }
	inline bool isCollectingStats(void) const { return flags & IS_COLLECTING; }
	inline bool logFor(log_level_t tested) const { return tested <= log_level; }
	inline log_level_t logLevel(void) const { return log_level; }

	// configuration
	void require(const AbstractFeature& feature);
	void provide(const AbstractFeature& feature);
	void invalidate(const AbstractFeature& feature);
	void use(const AbstractFeature& feature);

	// utilities
	void warn(const String& message);
	inline WorkSpace *workspace(void) const { return ws; }
	inline Progress& progress(void) { return *_progress; }
	void recordStat(const AbstractFeature& feature, StatCollector *collector);

	// cleanup
	inline void addCleaner(const AbstractFeature& feature, Cleaner *cleaner)
		{ cleaners.add(clean_t(&feature, cleaner)); }
	template <class T> void addRemover(const AbstractFeature& feature, const Ref<T, Identifier<T> >& ref)
		{ addCleaner(feature, new Remover<T>(ref)); }
	template <class T> void addDeletor(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }


	// Methods for customizing
	virtual void prepare(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void collectStats(WorkSpace *ws);

	// Deprecated
	virtual void processFrameWork(WorkSpace *fw);
	template <class T> T *track(const AbstractFeature& feature, T *object)
		{ addCleaner(feature, new elm::Deletor<T>(object)); return object; }
	template <class T> void track(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }
	template <class T> void track(const AbstractFeature& feature, const Ref<T *, const Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }

private:
	void init(const PropList& props);
	AbstractRegistration *_reg;
	WorkSpace *ws;
	typedef Pair<const AbstractFeature *, Cleaner *> clean_t;
	typedef elm::genstruct::SLList<clean_t> clean_list_t;
	clean_list_t cleaners;
	Progress *_progress;
	log_level_t log_level;
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

} // otawa

#endif // OTAWA_PROC_PROCESSOR_H
