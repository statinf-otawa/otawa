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
#include <otawa/proc/Processor.h>
#include "features.h"
#include "Config.h"

namespace otawa { namespace etime {

typedef Vector<Resource *> resources_t;
//typedef enum { IN_PREFIX = 0, IN_BLOCK = 1, IN_SIZE = 2 } part_t;
typedef int part_t;
const part_t IN_PREFIX = 0;
const part_t IN_BLOCK = 1;
const part_t IN_SIZE = 2;
class AbstractTimeBuilder;
class ILPGenerator;

class EventCase {
public:
	inline EventCase(void): _e(nullptr), _p(IN_SIZE), _i(-1) { }
	inline EventCase(Event *e, part_t p): _e(e), _p(p), _i(-1) { }

	inline Event *event() const { return _e; }
	inline part_t part() const { return _p; }
	inline int index() const { return _i; }
	inline void setIndex(int i) { _i = i; }
	inline Event *operator->() const { return _e; }
	inline Inst *inst() const { return _e->inst(); }
	inline bool isStatic() const { return _e->occurrence() == etime::ALWAYS || _e->occurrence() == etime::NEVER; }
	inline bool isDynamic() const { return !isStatic(); }

private:
	Event *_e;
	part_t _p;
	int _i;
};
io::Output& operator<<(io::Output& out, const EventCase& e);


class Factory {
	friend class AbstractTimeBuilder;
public:
	static Factory& def;
	virtual ~Factory(void);
	virtual ParExeGraph *make(ParExeProc *proc, Vector<Resource *> *hw_resources, ParExeSequence *seq) = 0;
	virtual ParExeNode *makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage) = 0;
	virtual ParExeEdge *makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type = ParExeEdge::SOLID, int latency = 0, string name = "") = 0;
};

class XGraphBuilder: public Monitor {
	friend class AbstractTimeBuilder;
public:
	static XGraphBuilder *make(Monitor& mon);
	XGraphBuilder(Monitor& mon);
	virtual ~XGraphBuilder();

	virtual ParExeGraph *build(ParExeSequence *seq) = 0;

protected:
	ParExeStage *findStage(cstring name) const;
	virtual void configure(const PropList& props);

	ParExeProc *processor(void) const;
	resources_t& resources(void) const;
	Factory *factory(void) const;
	bool isExplicit(void) const;

private:
	AbstractTimeBuilder *_atb;
};


class XGraphSolver: public Monitor {
	friend class AbstractTimeBuilder;
public:
	static XGraphSolver *make(Monitor& mon);
	XGraphSolver(Monitor& mon);
	virtual ~XGraphSolver(void);
	sys::Path dumpDir(void) const;

	virtual Factory *getFactory(void);
	virtual void compute(const PropList *entity, ParExeGraph *g, const Vector<EventCase>& events) = 0;

protected:
	virtual void configure(const PropList& props);

	void contributeBase(ot::time time);
	void contributeTime(ot::time time);
	void contributePositive(EventCase event, bool prec);
	void contributeNegative(EventCase event, bool prec);

private:
	AbstractTimeBuilder *_atb;
};


class ILPGenerator: public Monitor {
	friend class AbstractTimeBuilder;
	friend class XGraphSolver;
public:
	static ILPGenerator *make(Monitor& mon);
	ILPGenerator(Monitor& mon);
	virtual ~ILPGenerator(void);

	virtual void process(WorkSpace *ws) = 0;

protected:
	virtual void configure(const PropList& props);
	virtual void contributeBase(ot::time time) = 0;
	virtual void contributeTime(ot::time time) = 0;
	virtual void contributePositive(EventCase event, bool prec) = 0;
	virtual void contributeNegative(EventCase event, bool prec) = 0;

	inline AbstractTimeBuilder *builder(void) const { return _atb; }
	bool isExplicit(void) const;
	bool isRecording(void) const;
	ilp::System *system(void) const;
	ParExeGraph *build(ParExeSequence *seq);
	void solve(const PropList *entity, ParExeGraph *g, const Vector<EventCase>& events);
	void collectEvents(Vector<EventCase>& events, PropList *props, part_t part, p::id<Event *>& id);
	void collectPrologue(Vector<EventCase>& events, Block *v);
	void collectBlock(Vector<EventCase>& events, Block *v);
	void collectEdge(Vector<EventCase>& events, Edge *e);
	void sortEvents(Vector<EventCase>& events);
	int countDyn(const Vector<EventCase>& events);

private:
	AbstractTimeBuilder *_atb;
};

class AbstractTimeBuilder: public Processor {
	friend class XGraphBuilder;
	friend class XGraphSolver;
	friend class ILPGenerator;
public:
	static p::declare reg;
	AbstractTimeBuilder(p::declare& r = reg);

	inline XGraphSolver *solver(void) const { return _solver; }
	inline ILPGenerator *generator(void) const { return _generator; }
	inline XGraphBuilder *builder(void) const { return _builder; }
	void setBuilder(XGraphBuilder *builder);
	void setSolver(XGraphSolver *solver);
	void setGenerator(ILPGenerator *generator);

	void configure(const PropList& props) override;

protected:
	void setup(WorkSpace *ws) override;
	void processWorkSpace(WorkSpace *ws) override;
	void cleanup(WorkSpace *ws) override;

private:
	void prepareEvents(Vector<EventCase>& events);
	void processEdge(BasicBlock *src, Edge *e, BasicBlock *snk);
	void buildResources(void);

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
	ilp::System *_sys;
	bool _only_start;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_ABSTRACTTIMEBUILDER_H_ */
