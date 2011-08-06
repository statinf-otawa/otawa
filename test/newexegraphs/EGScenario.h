/*
 *	$Id$
 *	Interface to the EGScenario, EGScenarioBuilder classes.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#ifndef _EG_SCENARIO_H_
#define _EG_SCENARIO_H_

#include "ExecutionGraph.h"

#define TRACE(x) //x

namespace otawa { namespace exegraph2 {

class EGScenarioNodeItem {
private:
	EGNode * _node;
	uint32_t _latency;
public:
	EGScenarioNodeItem(EGNode *node, uint32_t latency)
	: _node(node), _latency(latency){}
	EGScenarioNodeItem(EGScenarioNodeItem& item)
	: _node(item.node()), _latency(item.latency()){}
	EGNode * node()
		{return _node;}
	uint32_t latency()
		{return _latency;}
	void dump(){
		elm::cerr << "[" << _node->name() << "=" << _latency << "]";
	}
};

class EGScenarioEdgeItem {
private:
	EGEdge * _edge;
	uint32_t _latency;
	bool _enabled;
public:
	EGScenarioEdgeItem(EGEdge *edge, uint32_t latency, bool enabled)
	: _edge(edge), _latency(latency), _enabled(enabled){}
	EGScenarioEdgeItem(EGScenarioEdgeItem& item)
	: _edge(item.edge()), _latency(item.latency()), _enabled(item.enabled()){}
	EGEdge * edge()
		{return _edge;}
	uint32_t latency()
		{return _latency;}
	bool enabled()
		{return _enabled;}
};

class EGScenario{
private:
	elm::genstruct::Vector<EGScenarioNodeItem*> _node_item_list;
	elm::genstruct::Vector<EGScenarioEdgeItem *> _edge_item_list;
public:
	EGScenario(){}
	EGScenario(EGScenario& scenario);
	~EGScenario();
	void addNodeItem(EGScenarioNodeItem *item)
		{_node_item_list.add(item);}
	void addEdgeItem(EGScenarioEdgeItem *item)
		{_edge_item_list.add(item);}
	void dump(){
		for (NodeItemIterator item(this); item ; item++){
			item->dump();
			elm::cerr << " -- ";
		}
		elm::cerr << " END\n";
	}

	class NodeItemIterator: public elm::genstruct::Vector<EGScenarioNodeItem *>::Iterator {
	public:
		inline NodeItemIterator(const EGScenario *scenario):
			elm::genstruct::Vector<EGScenarioNodeItem *>::Iterator(scenario->_node_item_list){}
	};
	class EdgeItemIterator: public elm::genstruct::Vector<EGScenarioEdgeItem *>::Iterator {
	public:
		inline EdgeItemIterator(const EGScenario *scenario):
			elm::genstruct::Vector<EGScenarioEdgeItem *>::Iterator(scenario->_edge_item_list){}
	};
};

class EGScenariiList{
private:
	elm::genstruct::Vector<EGScenario *> _list;
public:
	~EGScenariiList();
	void add(EGScenario *scenario)
		{_list.add(scenario);
		TRACE(
				elm::cerr << "[addScenario] adding scenario: ";
				scenario->dump();
				elm::cerr << " {" << scenario << "}\n";
				)
		}
	uint32_t length()
		{return _list.length();}
	void clear()
		{_list.clear();}
	void dump(){
		for (ScenarioIterator scenario(this) ; scenario; scenario++){
			scenario->dump();
			elm::cerr << "\n";
		}
		elm::cerr << " END\n";
	}
	class ScenarioIterator: public elm::genstruct::Vector<EGScenario *>::Iterator {
		public:
			inline ScenarioIterator(const EGScenariiList *list):
				elm::genstruct::Vector<EGScenario *>::Iterator(list->_list){}
		};
};

class EGScenarioBuilder{
protected:
	ExecutionGraph *_graph;
public:
	virtual EGScenariiList * build(WorkSpace *ws, ExecutionGraph * graph) = 0;
};

class EGGenericScenarioBuilder : public EGScenarioBuilder {
private:
	uint32_t _icache_hit_latency;
	uint32_t _icache_miss_latency;
public:
	EGScenariiList * build(WorkSpace *ws, ExecutionGraph * graph);
};

} // namespace exegraph2
} // namespace otawa
#endif // _EG_SCENARIOBUILDER_H_

