/*
 *	$Id$
 *	BHGDrawer class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#include "BBHGDrawer.h"
#include <otawa/cfg.h>
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>
#include <elm/genstruct/HashTable.h>


using namespace elm::genstruct;

namespace otawa { namespace bpred {


/**
 * @author G. Cavaignac
 * @class BBHGDrawer
 * This class uses the odisplay library to make a visual graph from a BBHG
 */


/**
 * Creates a new drawer, and makes the graph with all the data needed
 * @param bhg BBHG to print
 * @param graph configured Graph in which one wish to create the nodes and the edges
 */
BBHGDrawer::BBHGDrawer(BBHG *bhg, display::Graph *graph): _graph(graph), _made(false){
	_bhg = bhg;
}


/**
 * Creates a new drawer, and makes the graph with all the data needed
 * @param bhg BBHG to print
 * @param props properties to apply to all the items of the graph
 * (the Graph itself, Nodes, and Edges)
 * @param driver Driver to use to create the graph. The default is graphviz_driver
 */
BBHGDrawer::BBHGDrawer(BBHG *bhg, const PropList& props, display::Driver& driver): _made(false){
	PropList general, nodes, edges;
	general.addProps(props);
	nodes.addProps(props);
	edges.addProps(props);
	onInit(general, nodes, edges);
	
	_graph = driver.newGraph(general, nodes, edges);

	_bhg = bhg;
}


/**
 * Principal function. It creates Nodes and Edges to put in the Graph,
 * from the given BBHG
 * @param bhg source BBHG
 */
void BBHGDrawer::make(){
	if(_made){
		return;
	}
	ASSERT(_bhg);
	ASSERT(_graph);


	// Construct the Graph
	HashTable<void*, display::Node*> map;

	for(BBHG::Iterator bb(_bhg); bb; bb++){
		display::Node *node = _graph->newNode();
		map.put(*bb, node);
		onNode(*bb, node);
	}

	for(BBHG::Iterator bb(_bhg); bb; bb++){
		display::Node *node = map.get(*bb);
		for(BBHG::OutIterator succ(bb); succ; succ++){
			BBHGEdge* edge = succ;
			display::Edge *display_edge;
			display_edge = _graph->newEdge(node,map.get(edge->target()));
			onEdge(edge, display_edge);
		}
	}
	onEnd(_graph);
	_made = true;
}


/**
 * This function only displays the graph made.
 */
void BBHGDrawer::display(void){
	make();
	_graph->display();
}


/**
 * This function is called before creating the Graph.
 * One can give some properties to the PropLists
 * @param general PropList for the default behaviour and properties of the graph
 * @param nodes PropList for the default properties of nodes
 * @param edges PropList for the default properties of edges
 */
void BBHGDrawer::onInit(PropList& graph, PropList& nodes, PropList& edges){
	display::SHAPE(nodes) = display::ShapeStyle::SHAPE_MRECORD;
	display::FONT_SIZE(nodes) = 12;
	display::FONT_SIZE(edges) = 12;
	display::EXCLUDE(nodes).add(&INDEX);
}


/**
 * This function is called when a new Node is created.
 * One can give properties to the node
 * @param bb BasicBlock from which the node has been made
 * @param node Node made. One can give some properties to it
 */
void BBHGDrawer::onNode(BBHGNode *bb, otawa::display::Node *node){
	display::SHAPE(node) = display::ShapeStyle::SHAPE_MRECORD;

	// make title

	StringBuffer title;
	title << bb->getCorrespondingBB()->number() << ":";
	for(int i=bb->getHistory().size()-1;i>=0;--i)
		title << ((bb->getHistory().contains(i))?"1":"0");
	display::TITLE(node) = title.toString();
	StringBuffer body;
	if( bb->isEntry() || bb->isExit() ) {
		if(bb->isEntry()) body << "ENTRY ";
		if(bb->isExit()) {

			body << "EXIT :";
			if(bb->exitsWithNT()) body << "NT ";
			if(bb->exitsWithT()) body << "T";

		}
	}
	display::BODY(node) = body.toString();
}


/**
 * This function is called when a new Edge is created.
 * One can give properties to the edge
 * @param bhg_edge Edge of the BBHG from which the Edge of the Graph has been made
 * @param display_edge Edge of the Graph made. One can give properties to it
 */
void BBHGDrawer::onEdge(BBHGEdge *bhg_edge, otawa::display::Edge *display_edge){

	if(bhg_edge->isTaken()) {
		display::LABEL(display_edge) = "T";
	}
	else if(!bhg_edge->isFromBranch()){
		display::LABEL(display_edge) = "";
	}
	else {
		display::LABEL(display_edge) = "NT";
	}

}



/**
 * This function is called when the BBHG have been drawn.
 * One can add nodes, edges, or properties to the graph.
 * @param graph graph being drawn
 */
void BBHGDrawer::onEnd(otawa::display::Graph *graph){
}


/**
 * This function is called to display a node representing a called BBHG.
 */
void BBHGDrawer::onCall(BBHG *bhg, display::Node *node) {
	StringBuffer bf;
	bf << "classe @" << bhg->getClass(); 
	display::TITLE(node) = bf.toString();
}

} }		// otawa::bpred

