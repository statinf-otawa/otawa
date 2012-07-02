/*
 *	hard::ProcessorBuilder class interface
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
#ifndef OTAWA_HARD_PROCESSORBUILDER_H_
#define OTAWA_HARD_PROCESSORBUILDER_H_

#include <otawa/hard/Processor.h>
#include <elm/genstruct/Vector.h>

namespace otawa { namespace hard {

// FunctionalUnitBuilder class
class FunctionalUnitBuilder {
public:
	inline FunctionalUnitBuilder(string name): fu(new FunctionalUnit()) { fu->name = name; }
	inline FunctionalUnitBuilder& latency(int l) { fu->latency = l;  return *this; }
	inline FunctionalUnitBuilder& width(int w) { fu->width = w; return *this; }
	inline FunctionalUnitBuilder& pipelined(void) { fu->pipelined = true; return *this; }
	inline FunctionalUnit *operator*(void) { return fu; }
private:
	FunctionalUnit *fu;
};


// StageBuilder class
class StageBuilder {
public:
	inline StageBuilder(string name): stage(new Stage()), completed(false) { stage->name = name; }
	inline StageBuilder& type(Stage::type_t t) { stage->type = t; return *this; }
	inline StageBuilder& width(int w) { stage->width = w; return *this; }
	inline StageBuilder& latency(int l) { stage->latency = l; return *this; }
	inline StageBuilder& ordered(void) { stage->ordered = true; return *this; }
	inline StageBuilder& fu(FunctionalUnitBuilder& fu) { fus.add(*fu); return *this; }
	inline StageBuilder& dispatch(Inst::kind_t t, FunctionalUnitBuilder& fu) { Dispatch *d = new Dispatch(); disps.add(d); d->type = t; d->fu = *fu; return *this; }
	inline Stage *operator*(void) { complete(); return stage; }
private:
	Stage *stage;
	genstruct::Vector<FunctionalUnit *> fus;
	genstruct::Vector<Dispatch *> disps;
	bool completed;
	void complete(void);
};

// QueueBuilder class
class QueueBuilder {
public:
	inline QueueBuilder(string name): queue(new Queue()), completed(false) { queue->name = name; }
	inline QueueBuilder& input(StageBuilder& stage) { queue->input = *stage; return *this; }
	inline QueueBuilder& output(StageBuilder& stage) { queue->output = *stage; return *this; }
	inline QueueBuilder& intern(StageBuilder& stage) { interns.add(*stage); return *this; }
	inline QueueBuilder& size(int s) { queue->size = s; return *this; }
	inline Queue *operator*(void) { complete(); return queue; }

private:
	Queue *queue;
	genstruct::Vector<Stage *> interns;
	bool completed;
	void complete(void);
};

// ProcessorBuilder class
class ProcessorBuilder {
public:
	inline ProcessorBuilder(string model): proc(new Processor()), completed(false) { proc->model = model; }
	inline ProcessorBuilder& arch(string arch) { proc->arch = arch; return *this; }
	inline ProcessorBuilder& builder(string builder) { proc->builder = builder; return *this; }
	inline ProcessorBuilder& frequency(t::uint64 frequency) { proc->frequency = frequency; return *this; }
	inline ProcessorBuilder& add(StageBuilder& stage) { stages.add(*stage); return *this; }
	inline ProcessorBuilder& add(QueueBuilder& queue) { queues.add(*queue); return *this; }
	inline Processor *operator*(void) { complete(); return proc; }

private:
	Processor *proc;
	genstruct::Vector<Stage *> stages;
	genstruct::Vector<Queue *> queues;
	bool completed;
	void complete(void);
};

} }	// otawa::hard

#endif /* OTAWA_HARD_PROCESSORBUILDER_H_ */
