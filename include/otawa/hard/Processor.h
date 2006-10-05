/*
 *	$Id$
 *	Copyright (c) 2006, IRIT - UPS.
 *
 *	otawa/hard/Processor.h -- processor description interface.
 */
#ifndef OTAWA_HARD_PROCESSOR_H
#define OTAWA_HARD_PROCESSOR_H

#include <elm/string.h>
#include <elm/serial/interface.h>
#include <elm/serial/SerialTable.h>
#include <otawa/prog/Instruction.h>
#include <elm/util/StrongType.h>

namespace otawa { namespace hard {

// FunctionalUnit class
class FunctionalUnit {
	SERIALIZABLE
	elm::String name;
	int latency;
	int width;
	bool pipelined;
public:
	inline FunctionalUnit(void): latency(1), width(1), pipelined(false) { };
	inline elm::String getName(void) const { return name; };
	inline int getLatency(void) const { return latency; };
	inline int getWidth(void) const { return width; };
	inline bool isPipelined(void) const { return pipelined; };	
};

// Dispatch class
class Dispatch {
	SERIALIZABLE
public:
	typedef StrongType<Inst::kind_t> type_t;
private:
	type_t type;
	FunctionalUnit *fu;
public:
	inline Dispatch(void): type(0), fu(0) { };
	inline type_t getType(void) const { return type; };
	inline FunctionalUnit *getFU(void) const { return fu; };
};

// Stage class
class Stage {
	SERIALIZABLE
public:
	typedef enum type_t {
		NONE = 0,
		FETCH,
		LAZY,
		EXEC,
		COMMIT
	} type_t;
private:
	type_t type;
	elm::String name;
	int width;
	int latency;
	elm::serial::SerialTable<FunctionalUnit *> fus;
	elm::serial::SerialTable<Dispatch *> dispatch;
	bool ordered;
public:
	inline Stage(type_t _type = NONE): type(_type), width(1), latency(1), ordered(false) { };
	inline type_t getType(void) const { return type; };
	inline elm::String getName(void) const { return name; };
	inline int getWidth(void) const { return width; };
	inline int getLatency(void) const { return latency; };
	inline const elm::serial::SerialTable<FunctionalUnit *>&
		getFUs(void) const { return fus; };
	inline const elm::serial::SerialTable<Dispatch *>&
		getDispatch(void) const { return dispatch; };
	inline bool isOrdered(void) const { return ordered; };
};

// Queue class
class Queue {
	SERIALIZABLE
	elm::String name;
	int size;
	Stage *input, *output;
	elm::serial::SerialTable<Stage *> intern;
public:
	inline Queue(void): size(0), input(0), output(0) { };
	inline elm::String getName(void) const { return name; };
	inline int getSize(void) const { return size; };
	inline Stage *getInput(void) const { return input; };
	inline Stage *getOutput(void) const { return output; };	
	inline const elm::serial::SerialTable<Stage *>& getIntern(void) const { return intern; };
};

// Processor class
class Processor {
	SERIALIZABLE
	elm::String arch;
	elm::String model;
	elm::String builder;
	elm::serial::SerialTable<Stage *> stages;
	elm::serial::SerialTable<Queue *> queues;
public:
	inline elm::String getArch(void) const { return arch; };
	inline elm::String getModel(void) const { return model; };
	inline elm::String getBuilder(void) const { return builder; };
	inline const elm::genstruct::Table<Stage *>& getStages(void) const { return stages; };
	inline const elm::genstruct::Table<Queue *>& getQueues(void) const { return queues; };
};

} } // otawa::hard

SERIALIZABLE_ENUM(otawa::hard::Stage::type_t);

#endif // OTAWA_HARD_PROCESSOR_H
