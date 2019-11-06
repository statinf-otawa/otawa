/*
 *	EdgeTimeBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_ETIME_EDGETIMEBUILDER_H_
#define OTAWA_ETIME_EDGETIMEBUILDER_H_

#include <elm/data/HashMap.h>
#include <otawa/parexegraph/GraphBBTime.h>
#include <otawa/etime/EventCollector.h>
#include <otawa/etime/Config.h>
#include "features.h"

namespace otawa { namespace etime {

class ConfigSet;
class EdgeTimeBuilder;

class EdgeTimeGraph: public ParExeGraph {
public:
	EdgeTimeGraph(
		WorkSpace * ws,
		ParExeProc *proc,
		Vector<Resource *> *hw_resources,
		ParExeSequence *seq,const PropList& props = PropList::EMPTY);
	inline void setBuilder(const EdgeTimeBuilder& b) { builder = &b; }
protected:
	virtual void customDump(io::Output& out);
private:
	const EdgeTimeBuilder *builder;
};

class EdgeTimeBuilder: public GraphBBTime<EdgeTimeGraph> {
	friend class EdgeTimeGraph;
public:
	static p::declare reg;
	EdgeTimeBuilder(p::declare& r = reg);
	typedef enum { IN_PREFIX = 0, IN_EDGE = 1, IN_BLOCK = 2, IN_SIZE = 3 } place_t;

protected:
	virtual void configure(const PropList& props);

	// BBProcessor overload
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
	virtual void cleanup(WorkSpace *ws);

	// to customize
	typedef Pair<Event *, place_t> event_t;
	typedef Vector<event_t> event_list_t;
	typedef Vector<ConfigSet> config_list_t;
	virtual EdgeTimeGraph *make(ParExeSequence *seq);
	virtual void processEdge(WorkSpace *ws, CFG *cfg);
	virtual void processSequence(void);
	virtual void clean(ParExeGraph *graph);
	void processTimes(const config_list_t& confs);
	void applyStrictSplit(const config_list_t& confs);
	void applyFloppySplit(const config_list_t& confs);
	void applyWeightedSplit(const config_list_t& confs);

	// services
	static EventCollector::case_t make(const Event *e, EdgeTimeBuilder::place_t place, bool on);
	void contributeConst(void);
	void contributeSplit(const config_list_t& confs, t::uint32 pos, t::uint32 neg, t::uint32 com, ot::time lts_time, ot::time hts_time);
	void makeSplit(const config_list_t& confs, int p, ConfigSet& hts, ot::time& lts_time, ot::time& hts_time);
	inline Vector<Resource *> *ressources(void) { return &_hw_resources; }

private:

	class EventComparator {
	public:
		static int compare (const event_t& e1, const event_t& e2) {
			int res = e1.fst->inst()->address().compare(e2.fst->inst()->address());
			if (!res)
				res = e1.fst - e2.fst;
			return res;
		}
	};

	void apply(Event *event, ParExeInst *inst);
	void rollback(Event *event, ParExeInst *inst);
	EventCollector *get(Event *event);
	void genForOneCost(ot::time cost, Edge *edge, event_list_t& events);
	ParExeNode *getBranchNode(void);
	int splitConfs(const config_list_t& confs, const event_list_t& events, bool& lower);
	void sortEvents(event_list_t& events, BasicBlock *bb, place_t place, Edge *edge = 0);
	void displayConfs(const Vector<ConfigSet>& confs, const event_list_t& events);
	int countDynEvents(const event_list_t& events);

	ParExeInst *findInst(Inst *i, ParExeInst *from);
	ParExeNode *findNode(ParExeInst *i, const hard::PipelineUnit *unit);
	ParExeNode *findNode(Pair<Inst *, const hard::PipelineUnit *> loc, ParExeInst *i);
	void addLatency(ParExeNode *n, int l);
	void removeLatency(ParExeNode *n, int l);

	PropList _props;

	// ILP state
	bool _explicit;
	ilp::System *sys;
	bool predump;
	int event_th;

	// graph
	Edge *edge;
	Vector<event_t> all_events;
	event_list_t events;
	ParExeSequence *seq;
	EdgeTimeGraph *graph;
	ParExeNode *bnode;
	ParExeEdge *bedge;
	BasicBlock *source, *target;
	HashMap<Event *, ParExeEdge *> custom;

	// collector of events
	HashMap<Event *, EventCollector *> colls;

	// configuration
	bool record;
	t::uint32 event_mask;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_EDGETIMEBUILDER_H_ */
