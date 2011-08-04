/*
 *	$Id$
 *	 EGScenario, EGScenarioBuilder classes.
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


#include "EGScenario.h"
#include "EGBuilder.h"

using namespace otawa;
using namespace otawa::exegraph2;

EGScenario::EGScenario(EGScenario& scenario){
	for (NodeItemIterator item(&scenario) ; item ; item++){
		EGScenarioNodeItem * new_item = new EGScenarioNodeItem(*item.item());
		_node_item_list.add(new_item);
	}
	for (EdgeItemIterator item(&scenario) ; item ; item++){
			EGScenarioEdgeItem * new_item = new EGScenarioEdgeItem(*item.item());
			_edge_item_list.add(new_item);
		}
}

EGScenario::~EGScenario(){
	for (NodeItemIterator item(this) ; item ; item++)
		delete item;
	for (EdgeItemIterator item(this) ; item ; item++)
		delete item;
}

EGScenariiList::~EGScenariiList(){
	for (ScenarioIterator scenario(this) ; scenario ; scenario++)
		delete scenario;
}

EGScenariiList * EGGenericScenarioBuilder::build(WorkSpace *ws, ExecutionGraph * graph){
	_graph = graph;
	const otawa::hard::Cache *icache  = otawa::hard::CACHE_CONFIGURATION(ws)->instCache();
	if (icache){
		_icache_hit_latency = 1;
		_icache_miss_latency = icache->missPenalty();
	}
	else {
		_icache_hit_latency = 0;
		_icache_miss_latency = 0;
	}
	EGScenariiList *scenarii_list = new EGScenariiList();
	elm::genstruct::Vector<BasicBlock *> header_list;
	elm::genstruct::Vector<elm::genstruct::Vector<EGNode *> *> header_dependent_nodes_lists;
	elm::genstruct::Vector<EGNode *> variable_nodes_list;

	// classify unknown nodes : those that depend on the same loop header must be
	// considered together (with the same behavior)
	for (ExecutionGraph::UnknownNodeIterator u_node (_graph); u_node ; u_node++){
		if (u_node->stage()->category() == EGStage::FETCH){
			// latency depends on the icache behavior (FIRST_MISS or NOT_CLASSIFIED)
			LBlock *lb = LBLOCK(u_node->inst()->inst());
			ASSERT(lb);
			if (CATEGORY(lb) == FIRST_MISS){
				BasicBlock * header =CATEGORY_HEADER(lb);
				// has this header already been found?
				if (header_list.contains(header)){
					// yes: add u_node to the corresponding list
					int index = header_list.indexOf(header);
					header_dependent_nodes_lists[index]->add(u_node);
				}
				else {
					// no: create new header entry and list; add u_node to this list
					header_list.add(header);
					elm::genstruct::Vector<EGNode *> * new_list = new elm::genstruct::Vector<EGNode *>;
					header_dependent_nodes_lists.add(new_list);
					new_list->add(u_node);
				}
			}
			else { // not FIRST_MISS => NOT_CLASSIFIED
				variable_nodes_list.add(u_node);
			}
		}
		// other cases not handled so far
	}

	// build scenarii
	// start with an empty scenario
	EGScenario * empty_scenario = new EGScenario();
	scenarii_list->add(empty_scenario);

	// process header-related lists
	int num_headers = header_list.length();
	ASSERT(!num_headers || _icache_hit_latency);
	for (int index=0 ; index < num_headers ; index++){
		EGScenariiList to_add;
		for (EGScenariiList::ScenarioIterator scenario(scenarii_list); scenario ; scenario++){
			EGScenario *new_scenario = new EGScenario(*scenario.item());
			to_add.add(new_scenario);
			elm::genstruct::Vector<EGNode *> *list = header_dependent_nodes_lists[index];
			for (elm::genstruct::Vector<EGNode *>::Iterator node(*list); node ; node++){
				EGScenarioNodeItem *item = new EGScenarioNodeItem(node.item(), _icache_miss_latency);
				scenario->addNodeItem(item);
				item = new EGScenarioNodeItem(node.item(), _icache_hit_latency);
				new_scenario->addNodeItem(item);
			}
		}
		for (EGScenariiList::ScenarioIterator scenario(&to_add); scenario ; scenario++){
			scenarii_list->add(scenario.item());
		}
	}

	// process variable nodes list
	for (elm::genstruct::Vector<EGNode *>::Iterator node(variable_nodes_list); node ; node++){
		EGScenariiList to_add;
		for (EGScenariiList::ScenarioIterator scenario(scenarii_list); scenario ; scenario++){
			EGScenario *new_scenario = new EGScenario(*scenario.item());
			to_add.add(new_scenario);
			EGScenarioNodeItem *item = new EGScenarioNodeItem(node.item(), _icache_miss_latency);
			scenario->addNodeItem(item);
			item = new EGScenarioNodeItem(node.item(), _icache_hit_latency);
			new_scenario->addNodeItem(item);
		}
		for (EGScenariiList::ScenarioIterator scenario(&to_add); scenario ; scenario++){
			scenarii_list->add(scenario.item());
		}
	}

	// return scenarii list
	return scenarii_list;
}

