/*
 *	hard::Processor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-12, IRIT UPS.
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
#ifndef OTAWA_HARD_PROCESSOR_H
#define OTAWA_HARD_PROCESSOR_H

#include <elm/string.h>
#include <elm/serial2/macros.h>
#include <elm/serial2/collections.h>
#include <elm/genstruct/Table.h>
#include <otawa/prog/Inst.h>
#include <elm/util/strong_type.h>
#include <otawa/proc/SilentFeature.h>
#include <elm/system/Path.h>
#include <otawa/prog/Manager.h>

namespace otawa { namespace hard {

using namespace elm::genstruct;
class ProcessorBuilder;

// FunctionalUnit class
class FunctionalUnit {
	friend class FunctionalUnitBuilder;
	SERIALIZABLE(otawa::hard::FunctionalUnit,
		FIELD(name) & FIELD(latency) & FIELD(width) & FIELD(pipelined));
	elm::String name;
	int latency;
	int width;
	bool pipelined;
public:
	inline FunctionalUnit(void): latency(1), width(1), pipelined(false) { };
	virtual ~FunctionalUnit(void) { }
	inline elm::String getName(void) const { return name; };
	inline int getLatency(void) const { return latency; };
	inline int getWidth(void) const { return width; };
	inline bool isPipelined(void) const { return pipelined; };	
};

// Dispatch class
class Dispatch {
	friend class StageBuilder;
	SERIALIZABLE(otawa::hard::Dispatch, FIELD(type) & FIELD(fu));
public:
	STRONG_TYPE(type_t, Inst::kind_t); 
private:
	type_t type;
	FunctionalUnit *fu;
public:
	inline Dispatch(void): type(0), fu(0) { };
	virtual ~Dispatch(void) { }
	inline type_t getType(void) const { return type; };
	inline FunctionalUnit *getFU(void) const { return fu; };
};

// Stage class
class Stage {
	friend class StageBuilder;
	SERIALIZABLE(otawa::hard::Stage, FIELD(type) & FIELD(name) & FIELD(width)
		& FIELD(latency) & FIELD(fus) & FIELD(dispatch) & FIELD(ordered));
public:
	typedef enum type_t {
		NONE = 0,
		FETCH,
		LAZY,
		EXEC,
		COMMIT,
		DECOMP
	} type_t;
private:
	type_t type;
	elm::String name;
	int width;
	int latency;
	AllocatedTable<FunctionalUnit *> fus;
	AllocatedTable<Dispatch *> dispatch;
	bool ordered;
public:
	inline Stage(type_t _type = NONE): type(_type), width(1), latency(1), ordered(false) { };
	virtual ~Stage(void) { }
	inline type_t getType(void) const { return type; };
	inline elm::String getName(void) const { return name; };
	inline int getWidth(void) const { return width; };
	inline int getLatency(void) const { return latency; };
	inline const Table<FunctionalUnit *>& getFUs(void) const { return fus; }
	inline const Table<Dispatch *>& getDispatch(void) const { return dispatch; }
	inline bool isOrdered(void) const { return ordered; }
	template <class T> inline T select(Inst *inst, const T table[]) const; 
	template <class T> inline T select(Inst::kind_t kind, const T table[]) const; 
};

// Queue class
class Queue {
	friend class QueueBuilder;
	SERIALIZABLE(otawa::hard::Queue, FIELD(name) & FIELD(size) & FIELD(input)
		& FIELD(output) & FIELD(intern));
	elm::String name;
	int size;
	Stage *input, *output;
	AllocatedTable<Stage *> intern;
public:
	inline Queue(void): size(0), input(0), output(0) { }
	virtual ~Queue(void) { }
	inline elm::String getName(void) const { return name; }
	inline int getSize(void) const { return size; }
	inline Stage *getInput(void) const { return input; }
	inline Stage *getOutput(void) const { return output; }
	inline const AllocatedTable<Stage *>& getIntern(void) const
		{ return intern; }
};

// Processor class
class Processor {
	friend class ProcessorBuilder;
	SERIALIZABLE(otawa::hard::Processor, FIELD(arch) & FIELD(model)
		& FIELD(builder) & FIELD(stages) & FIELD(queues) & FIELD(frequency));

public:
	inline Processor(void): frequency(0) { }
	virtual ~Processor(void) { }
	inline elm::String getArch(void) const { return arch; };
	inline elm::String getModel(void) const { return model; };
	inline elm::String getBuilder(void) const { return builder; };
	inline const Table<Stage *>& getStages(void) const { return stages; };
	inline const Table<Queue *>& getQueues(void) const { return queues; };
	inline t::uint64 getFrequency(void) const { return frequency; }

	static const Processor null;
	static Processor *load(const elm::system::Path& path) throw(LoadException);
	static Processor *load(xom::Element *element) throw(LoadException);

private:
	elm::String arch;
	elm::String model;
	elm::String builder;
	AllocatedTable<Stage *> stages;
	AllocatedTable<Queue *> queues;
	t::uint64 frequency;
};


// Stage inlines
template <class T>
inline T Stage::select(Inst *inst, const T table[]) const {
	return select<T>(inst->kind(), table);
}

template <class T>
inline T Stage::select(Inst::kind_t kind, const T table[]) const {
	for(int i = 0; i < dispatch.count(); i++) {
		Inst::kind_t mask = dispatch[i]->getType();
		if((mask & kind) == mask)
			return table[i];
	}
}

// features
extern SilentFeature PROCESSOR_FEATURE;
extern Identifier<const Processor *> PROCESSOR;

} } // otawa::hard

ENUM(otawa::hard::Stage::type_t);
namespace elm { namespace serial2 {
	void __unserialize(Unserializer& s, otawa::hard::Dispatch::type_t& v);
	void __serialize(Serializer& s, otawa::hard::Dispatch::type_t v);
} } 

#endif // OTAWA_HARD_PROCESSOR_H
