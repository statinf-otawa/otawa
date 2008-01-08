#include "BBHGDrawer.h"
/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/BBHGDrawer.cpp -- display::BBHGDrawer class implementation.
 */
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>
#include <elm/genstruct/HashTable.h>


using namespace elm::genstruct;

namespace otawa { namespace display {


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
BBHGDrawer::BBHGDrawer(BBHG *bhg, Graph *graph): _graph(graph), _made(false){
	_bhg = bhg;
}


/**
 * Creates a new drawer, and makes the graph with all the data needed
 * @param bhg BBHG to print
 * @param props properties to apply to all the items of the graph
 * (the Graph itself, Nodes, and Edges)
 * @param driver Driver to use to create the graph. The default is graphviz_driver
 */
BBHGDrawer::BBHGDrawer(BBHG *bhg, const PropList& props, Driver& driver): _made(false){
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
	assert(_bhg);
	assert(_graph);


	// Construct the Graph
	HashTable<void*, Node*> map;

	for(BBHG::NodeIterator bb(_bhg); bb; bb++){
		Node *node = _graph->newNode();
		map.put(*bb, node);
		onNode(*bb, node);
	}

	for(BBHG::NodeIterator bb(_bhg); bb; bb++){
		Node *node = map.get(*bb);
		for(BBHG::Successor succ(bb); succ; succ++){
			BBHGEdge* edge = succ.edge();
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
	SHAPE(nodes) = ShapeStyle::SHAPE_MRECORD;
	FONT_SIZE(nodes) = 12;
	FONT_SIZE(edges) = 12;
	EXCLUDE(nodes).add(&INDEX);
}


/**
 * This function is called when a new Node is created.
 * One can give properties to the node
 * @param bb BasicBlock from which the node has been made
 * @param node Node made. One can give some properties to it
 */
void BBHGDrawer::onNode(BBHGNode *bb, otawa::display::Node *node){
	SHAPE(node) = ShapeStyle::SHAPE_MRECORD;

	// make title

	StringBuffer title;
	title << bb->getCorrespondingBB()->number() << ":";
	for(int i=bb->getHistory().size()-1;i>=0;--i)
		title << ((bb->getHistory().contains(i))?"1":"0");
	TITLE(node) = title.toString();
	StringBuffer body;
	if( bb->isEntry() || bb->isExit() ) {
		if(bb->isEntry()) body << "ENTRY ";
		if(bb->isExit()) {

			body << "EXIT :";
			if(bb->exitsWithNT()) body << "NT ";
			if(bb->exitsWithT()) body << "T";

		}
	}
	BODY(node) = body.toString();
}


/**
 * This function is called when a new Edge is created.
 * One can give properties to the edge
 * @param bhg_edge Edge of the BBHG from which the Edge of the Graph has been made
 * @param display_edge Edge of the Graph made. One can give properties to it
 */
void BBHGDrawer::onEdge(BBHGEdge *bhg_edge, otawa::display::Edge *display_edge){

	if(bhg_edge->isTaken()) {
		LABEL(display_edge) = "T";
	}
	else if(!bhg_edge->isFromBranch()){
		LABEL(display_edge) = "";
	}
	else {
		LABEL(display_edge) = "NT";
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
	TITLE(node) = bf.toString();
}

} }

