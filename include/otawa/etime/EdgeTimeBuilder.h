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

#include <elm/genstruct/HashTable.h>
#include <otawa/parexegraph/GraphBBTime.h>
#include <otawa/etime/EventCollector.h>
#include <otawa/etime/Config.h>
#include "features.h"

namespace otawa { namespace etime {

class ConfigSet;

class EdgeTimeBuilder: public GraphBBTime<ParExeGraph> {
public:
	static p::declare reg;
	EdgeTimeBuilder(p::declare& r = reg);
	typedef enum { IN_PREFIX = 0, IN_EDGE = 1, IN_BLOCK = 2, IN_SIZE = 3 } place_t;

protected:
	virtual void configure(const PropList& props);

	// BBProcessor overload
	virtual void setup(WorkSpace *ws);
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	virtual void cleanup(WorkSpace *ws);

	// to customize
	typedef Pair<Event *, place_t> event_t;
	typedef genstruct::Vector<event_t> event_list_t;
	typedef genstruct::Vector<ConfigSet> config_list_t;
	virtual ParExeGraph *make(ParExeSequence *seq);
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

private:

	class EventComparator {
	public:
		static int compare (const event_t& e1, const event_t& e2)
			{ return e1.fst->inst()->address().compare(e2.fst->inst()->address()); }
	};

	void processEdge(WorkSpace *ws, CFG *cfg);
	void apply(Event *event, ParExeInst *inst);
	void rollback(Event *event, ParExeInst *inst);
	EventCollector *get(Event *event);
	void genForOneCost(ot::time cost, Edge *edge, event_list_t& events);
	ParExeNode *getBranchNode(void);
	int splitConfs(const config_list_t& confs, const event_list_t& events, bool& lower);
	void sortEvents(event_list_t& events, BasicBlock *bb, place_t place, Edge *edge = 0);
	void displayConfs(const genstruct::Vector<ConfigSet>& confs, const event_list_t& events);

	// ILP state
	bool _explicit;
	ilp::System *sys;

	// graph
	ParExeNode *bnode;
	ParExeEdge *bedge;
	BasicBlock *source, *target;
	ParExeSequence *seq;
	ParExeGraph *graph;
	bool predump;

	// events
	Edge *edge;
	genstruct::Vector<event_t> all_events;
	event_list_t events;
	genstruct::HashTable<Event *, EventCollector *> colls;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_EDGETIMEBUILDER_H_ */
