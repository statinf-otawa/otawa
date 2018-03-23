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
typedef enum { IN_PREFIX = 0, IN_BLOCK = 1, IN_SIZE = 2 } part_t;
io::Output& operator<<(io::Output& out, part_t p);

class EventCase {
public:
	inline EventCase(void): _e(nullptr), _p(IN_SIZE), _i(-1) { }
	inline EventCase(Event *e, part_t p): _e(e), _p(p), _i(-1) { }

	inline Event *event(void) const { return _e; }
	inline part_t part(void) const { return _p; }
	inline int index(void) const { return _i; }
	inline void setIndex(int i) { _i = i; }

	inline Event *operator->(void) const { return _e; }

private:
	Event *_e;
	part_t _p;
	int _i;
};
io::Output& operator<<(io::Output& out, const EventCase& e);


class Factory {
public:
	virtual ~Factory(void);
	virtual ParExeGraph *make(ParExeProc *proc,  elm::genstruct::Vector<Resource *> *hw_resources, ParExeSequence *seq) = 0;
	virtual ParExeNode *makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage) = 0;
	virtual ParExeEdge *makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type = ParExeEdge::SOLID, int latency = 0, string name = "") = 0;
	static Factory *make(void);
};

class XGraphBuilder: public Monitor {
public:
	XGraphBuilder(const Monitor& mon);
	virtual ~XGraphBuilder(void);
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

	static XGraphBuilder *make(const Monitor& mon);

private:
	WorkSpace *_ws;
	ParExeProc *_processor;
	resources_t *_resources;
	Factory *_factory;
	bool _explicit;
};

class XGraphSolver: public Monitor {
public:
	XGraphSolver(const Monitor& mon);
	virtual ~XGraphSolver(void);
	Factory *getFactory(void);
	virtual void compute(ParExeGraph *g, List<ConfigSet *>& times, const Vector<EventCase>& events) = 0;
	inline sys::Path dumpDir(void) const { return _dir; }
	inline void setDumpDir(sys::Path dir) { _dir = dir; }
	static XGraphSolver *make(const Monitor& mon);
private:
	sys::Path _dir;
};

class ILPGenerator: public Monitor {
public:
	ILPGenerator(const Monitor& mon);
	virtual ~ILPGenerator(void);
	virtual void add(Edge *e, List<ConfigSet *> times, const Vector<EventCase>& events) = 0;
	virtual void complete(void) = 0;
	inline WorkSpace *workspace(void) const { return _ws; }
	inline ilp::System *system(void) const { return _sys; }
	inline bool isExplicit(void) const { return _explicit; }
	inline bool isRecording(void) const { return _recording; }
	inline void setWorkspace(WorkSpace *ws) { _ws = ws; }
	inline void setSystem(ilp::System *sys) { _sys = sys; }
	inline void setExplicit(bool exp) { _explicit = exp; }
	inline void setRecording(bool recording) { _recording = recording; }

	static ILPGenerator *make(const Monitor& mon);
private:
	WorkSpace *_ws;
	ilp::System *_sys;
	bool _explicit;
	bool _recording;
};

class AbstractTimeBuilder: public BBProcessor {
public:
	static p::declare reg;
	AbstractTimeBuilder(p::declare& r = reg);

	inline XGraphSolver *solver(void) const { return _solver; }
	inline ILPGenerator *generator(void) const { return _generator; }
	inline XGraphBuilder *builder(void) const { return _builder; }
	inline void setBuilder(XGraphBuilder *builder) { _builder = builder; }
	inline void setSolver(XGraphSolver *solver) { _solver = solver; }
	inline void setGenerator(ILPGenerator *generator) { _generator = generator; }

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
	void processSequence(Edge *e, ParExeSequence *seq, Vector<EventCase>& events);
	void displayTimes(const List<ConfigSet *>& confs, const Vector<EventCase>& events);

	XGraphBuilder *_builder;
	XGraphSolver *_solver;
	ILPGenerator *_generator;
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
