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

class EventCase {
public:
	inline EventCase(Event *e, code_part_t p): _e(e), _p(p), _i(-1) { }
	inline Event *event(void) const { return _e; }
	inline code_part_t part(void) const { return _p; }
	inline int index(void) const { return _i; }
	inline void setIndex(int i) { _i = i; }
private:
	Event *_e;
	code_part_t _p;
	int _i;
};

class Factory {
public:
	virtual ~Factory(void);
	virtual ParExeGraph *make(ParExeProc *proc,  elm::genstruct::Vector<Resource *> *hw_resources, ParExeSequence *seq) = 0;
	virtual ParExeNode *makeNode(ParExeInst *i, ParExeStage *stage) = 0;
	virtual ParExeEdge *makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type = ParExeEdge::SOLID, int latency = 0, string name = "") = 0;
	static Factory& DEFAULT;
};

class Builder {
public:
	virtual ~Builder(void);
	virtual ParExeGraph *build(ParExeSequence *seq) = 0;

	inline ParExeProc *processor(void) const { return _processor; }
	inline resources_t *resources(void) const { return _resources; }
	inline Factory *factory(void) const { return _factory; }
	inline void setProcessor(ParExeProc *processor) { _processor = processor; }
	inline void setResources(resources_t *resources) { _resources = resources; }
	inline void setFactory(Factory *factory) { _factory = factory; }

	static Builder& builder;

private:
	ParExeProc *_processor;
	resources_t *_resources;
	Factory *_factory;
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
	virtual void generate(ilp::System *sys, ParExeGraph *g);
	static Generator& DEFAULT;
};

class AbstractTimeBuilder: public BBProcessor {
public:
	static p::declare reg;
	AbstractTimeBuilder(p::declare& r = reg);

	inline Engine *engine(void) const { return _engine; }
	inline Generator *generator(void) const { return _generator; }
	inline void setEngine(Engine *engine) { _engine = engine; }

protected:
	void setup(WorkSpace *ws) override;
	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override;
	void cleanup(WorkSpace *ws) override;

private:
	Engine *_engine;
	Generator *_generator;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_ABSTRACTTIMEBUILDER_H_ */
