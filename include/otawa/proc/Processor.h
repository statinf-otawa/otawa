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

#include <elm/data/List.h>
#include <elm/io.h>
#include <elm/util/Cleaner.h>
#include <elm/util/Version.h>
#include <elm/sys/StopWatch.h>
#include <elm/data/Vector.h>
#include <elm/data/HashMap.h>
#include <elm/util/Cleaner.h>

#include <otawa/proc/ProcessorException.h>
#include <otawa/proc/Registration.h>
#include <otawa/proc/Monitor.h>
#include "../prop.h"

namespace otawa {

using namespace elm;
class AbstractFeature;
class Configuration;
class WorkSpace;
class FeatureDependency;
class Progress;
class StatCollector;


// Processor class
class Processor: public otawa::Monitor {
	friend class WorkSpace;

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
	static Processor& null;

	// Constructors
	Processor(void);
	Processor(AbstractRegistration& registration);
	Processor(String name, Version version);
	Processor(String name, Version version, AbstractRegistration& registration);
	virtual ~Processor(void);
	static p::declare reg;

	// Accessors
	inline elm::String name(void) const { return _reg->name(); }
	inline elm::Version version(void) const { return _reg->version(); }
	inline AbstractRegistration& registration(void) const { return *_reg; }
	virtual void *interfaceFor(const AbstractFeature& feature);

	// Mutators
	virtual void configure(const PropList& props);

	// Configuration Properties
	static p::id<PropList *> STATS;
	static p::id<bool> TIMED;
	static p::id<bool> RECURSIVE;
	static p::id<Progress *> PROGRESS;
	static p::id<bool> COLLECT_STATS;

	// deprecated
	static p::id<elm::io::OutStream *>& OUTPUT;
	static p::id<elm::io::OutStream *>& LOG;
	static p::id<bool>& VERBOSE;
	static p::id<log_level_t>& LOG_LEVEL;
	static p::id<string>& LOG_FOR;

	// Statistics Properties
	static p::id<elm::sys::time_t> RUNTIME;

	// Deprecated
	Processor(const PropList& props);
	Processor(elm::String name, elm::Version version, const PropList& props);

protected:
	static const t::uint32
		IS_TIMED		= 0x01 << CUSTOM_SHIFT,
		IS_ALLOCATED	= 0x04 << CUSTOM_SHIFT,
		IS_PREPARED		= 0x08 << CUSTOM_SHIFT,
		IS_COLLECTING	= 0x10 << CUSTOM_SHIFT,
		IS_TIED			= 0x20 << CUSTOM_SHIFT,
		IS_DONE			= 0x40 << CUSTOM_SHIFT;
	PropList *stats;

	// accessors
	friend class FeatureRequirer;
	inline bool isTimed(void) const { return flags & IS_TIMED; }
	inline bool recordsStats(void) const { return stats; }
	inline bool isAllocated(void) const { return flags & IS_ALLOCATED; }
	inline bool isPrepared(void) const { return flags & IS_PREPARED; }
	inline bool isCollectingStats(void) const { return flags & IS_COLLECTING; }
	inline bool isDone() const { return flags & IS_DONE; }

	// configuration
	void require(const AbstractFeature& feature);
	void provide(const AbstractFeature& feature);
	void invalidate(const AbstractFeature& feature);
	void use(const AbstractFeature& feature);

	// utilities
	void warn(const String& message);
	inline WorkSpace *workspace(void) const { return ws; }
	inline Progress& progress(void) { return *_progress; }
	void record(StatCollector *collector);
	void track(Cleaner *cleaner);
	template <class T> void track(const Ref<T, const Identifier<T> >& ref)
		{ track(new Remover<T>(ref)); }
	template <class T> void track(const Ref<T *, const Identifier<T *> >& ref)
		{ track(new Deletor<T>(ref)); }
	template <class T> T *track(T *object)
		{ track(static_cast<Cleaner *>(new elm::Deletor<T>(object))); return object; }

	// Methods for customizing
	virtual void prepare(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	virtual void collectStats(WorkSpace *ws);
	virtual void commit(WorkSpace *ws);
	virtual void destroy(WorkSpace *ws);

	// deprecated
	void recordStat(const AbstractFeature& feature, StatCollector *collector);
	inline void addCleaner(const AbstractFeature& feature, Cleaner *cleaner)
		{ cleaners.add(cleaner); }
	template <class T> void addRemover(const AbstractFeature& feature, const Ref<T, Identifier<T> >& ref)
		{ addCleaner(feature, new Remover<T>(ref)); }
	template <class T> void addDeletor(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }
	template <class T> T *track(const AbstractFeature& feature, T *object)
		{ addCleaner(feature, new elm::Deletor<T>(object)); return object; }
	template <class T> void track(const AbstractFeature& feature, const Ref<T *, Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }
	template <class T> void track(const AbstractFeature& feature, const Ref<T *, const Identifier<T *> >& ref)
		{ addCleaner(feature, new Deletor<T>(ref)); }
	template <class T> void track(const AbstractFeature& feature, const Ref<T, Identifier<T> >& ref)
		{ addCleaner(feature, new Remover<T>(ref)); }
	template <class T> void track(const AbstractFeature& feature, const Ref<T, const Identifier<T> >& ref)
		{ addCleaner(feature, new Remover<T>(ref)); }

	// internal use only
	virtual void requireDyn(WorkSpace *ws, const PropList& props);

private:
	void init(const PropList& props);
	void run(WorkSpace *ws);

	AbstractRegistration *_reg;
	WorkSpace *ws;
	List<Cleaner *> cleaners;
	Progress *_progress;
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
public:
	String message() override;
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
