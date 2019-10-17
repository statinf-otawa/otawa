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

#include <elm/data/Array.h>
#include <elm/serial2/macros.h>
#include <elm/serial2/collections.h>
#include <elm/string.h>
#include <elm/sys/Path.h>
#include <elm/util/strong_type.h>
#include <otawa/hard/features.h>
#include <otawa/prog/Inst.h>
#include <otawa/prog/Manager.h>

namespace otawa {

class Process;

namespace hard {

using namespace elm;

// PipelineUnit class
class PipelineUnit {
	friend class Processor;
	SERIALIZABLE(otawa::hard::PipelineUnit,
		FIELD(name) &
		FIELD(latency) &
		FIELD(width) &
		FIELD(branch) &
		FIELD(mem) &
		FIELD(mem_stage));

public:
	class Make {
	public:
		friend class PipelineUnit;
		inline Make(string name = "")
			: _name(name), _latency(1), _width(1), branch(false), mem(false), mem_stage(0) { }
		inline Make& latency(int l) { _latency = l; return *this; }
		inline Make& width(int w) { _width =w; return *this; }
		inline Make& do_branch(void) { branch = true; return *this; }
		inline Make& access_mem(void) { mem = true; return *this; }
	private:
		string _name;
		int _latency, _width;
		bool branch, mem;
		int mem_stage;
	};

	PipelineUnit();
	PipelineUnit(const Make& maker);
	PipelineUnit(const PipelineUnit *unit);
	virtual ~PipelineUnit();
	inline elm::String getName() const { return name; };
	inline int getLatency() const { return latency; };
	inline int getWidth() const { return width; };
	inline bool isBranch() const { return branch; }
	inline bool isMem() const { return mem; }
	inline int memStage() const { return mem_stage; }
	inline int index() const { return _index; }

protected:
	string name;
	int latency;
	int width;
	bool branch;
	bool mem;
	int mem_stage;
	int _index;
};

// FunctionalUnit class
class FunctionalUnit: public PipelineUnit {
	friend class FunctionalUnitBuilder;
	SERIALIZABLE(otawa::hard::FunctionalUnit, BASE(otawa::hard::PipelineUnit) & FIELD(pipelined));

public:

	class Make: private PipelineUnit::Make {
	public:
		friend class FunctionalUnit;
		inline Make(string name = "")
			: PipelineUnit::Make(name), _pipelined(true) { }
		inline Make& latency(int l) { PipelineUnit::Make::latency(l); return *this; }
		inline Make& width(int w) { PipelineUnit::Make::width(w); return *this; }
		inline Make& do_branch(void) { PipelineUnit::Make::do_branch(); return *this; }
		inline Make& access_mem(void) { PipelineUnit::Make::access_mem(); return *this; }
		inline Make& is_pipelined(void) { _pipelined = true; return *this; }
	private:
		bool _pipelined;
	};

	FunctionalUnit(void);
	FunctionalUnit(const Make& maker);
	FunctionalUnit(const FunctionalUnit *fu);
	virtual ~FunctionalUnit(void);
	inline bool isPipelined(void) const { return pipelined; };	

private:
	bool pipelined;
};

// Dispatch class
class Dispatch {
	friend class StageBuilder;
	SERIALIZABLE(otawa::hard::Dispatch, FIELD(type) & FIELD(fu));

public:
	STRONG_TYPE(type_t, Inst::kind_t); 
	Dispatch(type_t t, FunctionalUnit *f);
	Dispatch(void);
	virtual ~Dispatch(void);
	inline type_t getType(void) const { return type; };
	inline FunctionalUnit *getFU(void) const { return fu; };

private:
	type_t type;
	FunctionalUnit *fu;
};

// Stage class
class Stage: public PipelineUnit {
	friend class StageBuilder;
	SERIALIZABLE(otawa::hard::Stage, BASE(otawa::hard::PipelineUnit) & FIELD(type) & FIELD(fus) & FIELD(dispatch) & FIELD(ordered));
public:
	typedef enum type_t {
		NONE = 0,
		FETCH,
		LAZY,
		EXEC,
		COMMIT,
		DECOMP
	} type_t;

	class Make: private PipelineUnit::Make {
	public:
		friend class Stage;
		inline Make(string name = "", type_t type = NONE)
			: PipelineUnit::Make(name), _type(type), _ordered(true) { }
		inline Make& latency(int l) { PipelineUnit::Make::latency(l); return *this; }
		inline Make& width(int w) { PipelineUnit::Make::width(w); return *this; }
		inline Make& do_branch(void) { PipelineUnit::Make::do_branch(); return *this; }
		inline Make& access_mem(void) { PipelineUnit::Make::access_mem(); return *this; }
		inline Make& is_ordered(void) { _ordered = true; return *this; }
		inline Make& add(FunctionalUnit *fu) { fus.add(fu); return *this; }
		inline Make& add(FunctionalUnit& fu) { fus.add(&fu); return *this; }
		inline Make& add(const FunctionalUnit::Make& m) { fus.add(new FunctionalUnit(m)); return *this; }
		inline Make& dispatch(Dispatch::type_t t, FunctionalUnit *f) { dispatches.add(new Dispatch(t, f)); return *this; }
	private:
		type_t _type;
		bool _ordered;
		Vector<FunctionalUnit *> fus;
		Vector<Dispatch *> dispatches;
	};

	Stage(type_t _type = NONE);
	Stage(const Make& maker);
	Stage(const Stage *stage);
	virtual ~Stage(void);

	inline type_t getType(void) const { return type; };
	inline const Array<FunctionalUnit *>& getFUs(void) const { return fus; }
	inline const Array<Dispatch *>& getDispatch(void) const { return dispatch; }
	inline bool isOrdered(void) const { return ordered; }
	const PipelineUnit *select(Inst *inst) const;

	// deprecated
	template <class T> inline T select(Inst *inst, const T table[]) const; 
	template <class T> inline T select(Inst::kind_t kind, const T table[]) const; 

private:
	type_t type;
	AllocArray<FunctionalUnit *> fus;
	AllocArray<Dispatch *> dispatch;
	bool ordered;
};

// Queue class
class Queue {
	friend class Processor;
	friend class QueueBuilder;
	SERIALIZABLE(otawa::hard::Queue, FIELD(name) & FIELD(size) & FIELD(input)
		& FIELD(output) & FIELD(intern));

public:

	class Make {
		friend class Queue;
	public:
		inline Make(string name): _name(name), _size(1), _input(0), _output(0) { }
		inline Make& size(int s) { _size = s; return *this; }
		inline Make& input(Stage *s) { _input = s; return *this; }
		inline Make& output(Stage *s) { _output = s; return *this; }
		inline Make& intern(Stage *s) { _intern.add(s); return *this; }
		inline Make& input(Stage& s) { _input = &s; return *this; }
		inline Make& output(Stage& s) { _output = &s; return *this; }
		inline Make& intern(Stage& s) { _intern.add(&s); return *this; }
	private:
		string _name;
		int _size;
		Stage *_input, *_output;
		Vector<Stage *> _intern;
	};

	Queue(void);
	Queue(const Make& make);
	virtual ~Queue(void);
	inline elm::String getName(void) const { return name; }
	inline int getSize(void) const { return size; }
	inline Stage *getInput(void) const { return input; }
	inline Stage *getOutput(void) const { return output; }
	inline const AllocArray<Stage *>& getIntern(void) const { return intern; }
	inline int index() const { return _index; }

private:
	elm::String name;
	int size;
	Stage *input, *output;
	AllocArray<Stage *> intern;
	int _index;
};


// Step description
class Step {
public:
	typedef enum {
		NONE,
		STAGE,
		READ,
		WRITE,
		USE,
		RELEASE,
		BRANCH,
		ISSUE_MEM,
		WAIT_MEM,
		WAIT
	} kind_t;

	inline Step(void): _kind(NONE) { }
	inline Step(const PipelineUnit *unit): _kind(STAGE) { arg.unit = unit; }
	inline Step(kind_t kind, const hard::Register *reg): _kind(kind) { ASSERT(kind == READ || kind == WRITE); arg.reg = reg; }
	inline Step(kind_t kind, hard::Queue *queue): _kind(kind) { ASSERT(kind == USE || kind == RELEASE); arg.queue = queue; }
	inline Step(kind_t kind): _kind(kind) { ASSERT(kind == BRANCH); }
	inline Step(bool store, bool cached = true): _kind(ISSUE_MEM)
		{ arg.mem.store = store; arg.mem.cached = cached; }
	inline Step(kind_t kind, int delay): _kind(kind) { ASSERT(_kind == WAIT || _kind == WAIT_MEM); arg.delay = delay; }

	inline kind_t kind(void) const { return _kind; }
	inline const PipelineUnit *stage(void) const { ASSERT(_kind == STAGE); return arg.unit; }
	inline const Register *reg(void) const { ASSERT(_kind == READ || _kind == WRITE); return arg.reg; }
	inline Queue *queue(void) const { ASSERT(_kind == USE || _kind == RELEASE); return arg.queue; }
	inline bool isCached() const { ASSERT(_kind == ISSUE_MEM); return arg.mem.cached; }
	inline bool isLoad() const { ASSERT(_kind == ISSUE_MEM); return !arg.mem.store; }
	inline bool isStore() const { ASSERT(_kind == ISSUE_MEM); return arg.mem.store; }
	inline int delay() const { ASSERT(_kind == WAIT || _kind == WAIT_MEM); return arg.delay; }

private:
	kind_t _kind;
	union {
		const PipelineUnit *unit;
		const Register *reg;
		Queue *queue;
		struct {
			bool store;
			bool cached;
		} mem;
		int delay;
	} arg;
};
Output& operator<<(Output& out, const Step& step);


// Processor class
class Processor: public AbstractIdentifier {
	friend class ProcessorProcessor;
	SERIALIZABLE(otawa::hard::Processor,
		FIELD(arch) &
		FIELD(model) &
		FIELD(builder) &
		FIELD(stages) &
		FIELD(queues) &
		FIELD(frequency));

public:

	class Make {
		friend class Processor;
	public:
		inline Make(string arch): _arch(arch), _frequency(0), pf(nullptr) { }
		inline Make& model(string m) { _model = m; return *this; }
		inline Make& builder(string b) { _builder = b; return *this; }
		inline Make& frequency(t::uint64 f) { _frequency = f; return *this; }
		inline Make& platform(hard::Platform *p) { pf = p; return *this; }
		inline Make& platform(string name) { pf = Platform::find(name); return *this; }
		inline Make& add(Stage *s) { stages.add(s); return *this; }
		inline Make& add(Stage& s) { stages.add(&s); return *this; }
		inline Make& add(Stage::Make& m) { stages.add(new Stage(m)); return *this; }
		inline Make& add(Queue *q) { queues.add(q); return *this; }
		inline Make& add(Queue& q) { queues.add(&q); return *this; }
		inline Make& add(Queue::Make& m) { queues.add(new Queue(m)); return *this; }
	private:
		string _arch, _model, _builder;
		t::uint64 _frequency;
		Vector<Stage *> stages;
		Vector<Queue *> queues;
		hard::Platform *pf;
	};

	Processor();
	Processor(const Make& m, cstring name = "");
	Processor(const Processor& proc, cstring name = "");
	virtual ~Processor();

	inline elm::String getArch() const { return arch; };
	inline elm::String getModel() const { return model; };
	inline elm::String getBuilder() const { return builder; };
	inline const Array<Stage *>& getStages(void) const { return stages; };
	inline const Array<Queue *>& getQueues(void) const { return queues; };
	inline t::uint64 getFrequency() const { return frequency; }

	inline hard::Platform *platform() const { return _pf; }
	inline int unitCount() const { return _unit_count; }
	inline int queueCount() const { return queues.count(); }

	static const Processor null;
	static Processor *load(const elm::sys::Path& path);
	static Processor *load(xom::Element *element);

	typedef Vector<Step> steps_t;
	virtual void execute(Inst *inst, steps_t& steps) const;
	virtual Processor *clone(cstring name = "") const;
	virtual Processor *instantiate(Process *process, cstring name = "") const;

	inline void __serial_complete() { init(); }

protected:
	inline Process *process(void) const { return _process; }

private:
	void init();
	string arch;
	string model;
	string builder;
	AllocArray<Stage *> stages;
	AllocArray<Queue *> queues;
	t::uint64 frequency;
	Process *_process;
	hard::Platform *_pf;
	int _unit_count;
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
extern Identifier<string> PROCESSOR_ID;

} } // otawa::hard

ENUM(otawa::hard::Stage::type_t);
namespace elm { namespace serial2 {
	void __unserialize(Unserializer& s, otawa::hard::Dispatch::type_t& v);
	void __serialize(Serializer& s, otawa::hard::Dispatch::type_t v);
} } 

#endif // OTAWA_HARD_PROCESSOR_H
