/*
 *	$Id$
 *	Copyright (c) 2006, IRIT - UPS.
 *
 *	otawa/hard/Processor.h -- processor description interface.
 */
#ifndef OTAWA_HARD_PROCESSOR_H
#define OTAWA_HARD_PROCESSOR_H

#include <elm/string.h>
#include <elm/serial2/macros.h>
#include <elm/serial2/collections.h>
#include <elm/genstruct/Table.h>
#include <otawa/prog/Inst.h>
#include <elm/util/strong_type.h>

namespace otawa { namespace hard {

using namespace elm::genstruct;

// FunctionalUnit class
class FunctionalUnit {
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
	SERIALIZABLE(otawa::hard::Processor, FIELD(arch) & FIELD(model)
		& FIELD(builder) & FIELD(stages) & FIELD(queues));
	elm::String arch;
	elm::String model;
	elm::String builder;
	AllocatedTable<Stage *> stages;
	AllocatedTable<Queue *> queues;
public:
	virtual ~Processor(void) { }
	inline elm::String getArch(void) const { return arch; };
	inline elm::String getModel(void) const { return model; };
	inline elm::String getBuilder(void) const { return builder; };
	inline const Table<Stage *>& getStages(void) const { return stages; };
	inline const Table<Queue *>& getQueues(void) const { return queues; };
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

} } // otawa::hard

ENUM(otawa::hard::Stage::type_t);
namespace elm { namespace serial2 {
	void __unserialize(Unserializer& s, otawa::hard::Dispatch::type_t& v);
	void __serialize(Serializer& s, otawa::hard::Dispatch::type_t v);
} } 

#endif // OTAWA_HARD_PROCESSOR_H
