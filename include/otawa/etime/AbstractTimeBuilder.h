/*
 *	EdgeTimeBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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
#ifndef OTAWA_ETIME_ABSTRACTTIMEBUILDER_H_
#define OTAWA_ETIME_ABSTRACTTIMEBUILDER_H_

#include <elm/data/List.h>
#include <otawa/parexegraph/ParExeGraph.h>
#include <otawa/proc/BBProcessor.h>
#include "features.h"
#include "Config.h"

namespace otawa { namespace etime {

typedef elm::genstruct::Vector<Resource *> resources_t;
typedef enum { IN_PREFIX = 0, IN_EDGE = 1, IN_BLOCK = 2, IN_SIZE = 3 } part_t;

class EventCase {
public:
	inline EventCase(void): _e(nullptr), _p(IN_SIZE), _i(-1) { }
	inline EventCase(Event *e, part_t p): _e(e), _p(p), _i(-1) { }
	inline Event *event(void) const { return _e; }
	inline part_t part(void) const { return _p; }
	inline int index(void) const { return _i; }
	inline void setIndex(int i) { _i = i; }
private:
	Event *_e;
	part_t _p;
	int _i;
};

class Factory {
public:
	virtual ~Factory(void);
	virtual ParExeGraph *make(ParExeProc *proc,  elm::genstruct::Vector<Resource *> *hw_resources, ParExeSequence *seq) = 0;
	virtual ParExeNode *makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage) = 0;
	virtual ParExeEdge *makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type = ParExeEdge::SOLID, int latency = 0, string name = "") = 0;
	static Factory *make(void);
};

class Builder {
public:
	Builder(void);
	virtual ~Builder(void);
	virtual ParExeGraph *build(ParExeSequence *seq) = 0;

	inline WorkSpace *workspace(void)  const { return _ws; }
	inline ParExeProc *processor(void) const { return _processor; }
	inline resources_t *resources(void) const { return _resources; }
	inline Factory *factory(void) const { return _factory; }
	inline bool isExplicit(void) const { return _explicit; }
	inline void setWorkSpace(WorkSpace *ws) { _ws = ws; }
	inline void setProcessor(ParExeProc *processor) { _processor = processor; }
	inline void setResources(resources_t *resources) { _resources = resources; }
	inline void setFactory(Factory *factory) { _factory = factory; }
	inline void setExplicit(bool is_explicit) { _explicit = is_explicit; }

	static Builder& DEFAULT;

private:
	WorkSpace *_ws;
	ParExeProc *_processor;
	resources_t *_resources;
	Factory *_factory;
	bool _explicit;
};

class Engine {
public:
	virtual ~Engine(void);
	Factory *getFactory(void);
	virtual void compute(ParExeGraph *g, List<ConfigSet *> times, const Vector<EventCase>& events) = 0;
	static Engine& DEFAULT;
};

class Generator {
public:
	virtual ~Generator(void);
	virtual void add(ParExeGraph *g, List<ConfigSet *> times) = 0;
	virtual void complete(void) = 0;
	inline WorkSpace *workspace(void) const { return _ws; }
	inline ilp::System *system(void) const { return _sys; }
	inline void setWorkspace(WorkSpace *ws) { _ws = ws; }
	inline void setSystem(ilp::System *sys) { _sys = sys; }
	static Generator& DEFAULT;
private:
	WorkSpace *_ws;
	ilp::System *_sys;
};

class AbstractTimeBuilder: public BBProcessor {
public:
	static p::declare reg;
	AbstractTimeBuilder(p::declare& r = reg);

	inline Engine *engine(void) const { return _engine; }
	inline Generator *generator(void) const { return _generator; }
	inline Builder *builder(void) const { return _builder; }
	inline void setBuilder(Builder *builder) { _builder = builder; }
	inline void setEngine(Engine *engine) { _engine = engine; }
	inline void setGenerator(Generator *generator) { _generator = generator; }

	void configure(const PropList& props) override;

protected:
	void setup(WorkSpace *ws) override;
	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override;
	void cleanup(WorkSpace *ws) override;

private:
	void processEdge(BasicBlock *src, Edge *e, BasicBlock *snk);
	void buildResources(void);
	void collectEvents(Vector<EventCase>& events, PropList *props, part_t part, p::id<Event *>& id);
	int countDynEvents(const Vector<EventCase>& events);
	void processSequence(ParExeSequence *seq, Vector<EventCase>& events);
	void displayTimes(const List<ConfigSet *>& confs, const Vector<EventCase>& events);


	Builder *_builder;
	Engine *_engine;
	Generator *_generator;
	ParExeProc *_proc;
	resources_t _resources;

	bool _explicit;
	bool _predump;
	int _event_th;
	bool _record;
	sys::Path _dir;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_ABSTRACTTIMEBUILDER_H_ */
