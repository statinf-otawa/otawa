/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/CFGDrawer.cpp -- display::CFGDrawer class implementation.
 */
#include <otawa/display/Driver.h>
#include <otawa/display/graphviz.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/display/CFGDrawer.h>

using namespace elm::genstruct;

namespace otawa { namespace display {


/**
 * @author G. Cavaignac
 * @class CFGDrawer
 * This class uses the odisplay library to make a visual graph from a CFG
 * @deprecated	Use instead CFGOutput that provides a lot of customizations.
 */


/**
 * Creates a new drawer, and makes the graph with all the data needed
 * @param cfg CFG to print
 * @param graph configured Graph in which one wish to create the nodes and the edges
 */
CFGDrawer::CFGDrawer(CFG *cfg, Graph *graph): _graph(graph), _made(false){
	//make(cfg);
	_cfg = cfg;
}


/**
 * Creates a new drawer, and makes the graph with all the data needed
 * @param cfg CFG to print
 * @param props properties to apply to all the items of the graph
 * (the Graph itself, Nodes, and Edges)
 * @param driver Driver to use to create the graph. The default is graphviz_driver
 */
CFGDrawer::CFGDrawer(CFG *cfg, const PropList& props, Driver& driver): _made(false){
	PropList general, nodes, edges;
	general.addProps(props);
	nodes.addProps(props);
	edges.addProps(props);
	onInit(general, nodes, edges);

	_graph = driver.newGraph(general, nodes, edges);
	_graph->setProps(*cfg);
	//make(cfg);
	_cfg = cfg;
}


/**
 * Principal function. It creates Nodes and Edges to put in the Graph,
 * from the given CFG
 * @param cfg source CFG
 */
void CFGDrawer::make(){
	if(_made){
		return;
	}
	ASSERT(_cfg);
	ASSERT(_graph);

	// Construct the Graph
	HashTable<void*, Node*> map;
	for(CFG::BBIterator bb(_cfg); bb; bb++){
		Node *node = _graph->newNode();
		map.put(*bb, node);
		onNode(*bb, node);
	}
	for(CFG::BBIterator bb(_cfg); bb; bb++){
		Node *node = map.get(*bb);
		for(BasicBlock::OutIterator edge(bb); edge; edge++){
			if(edge->kind() != otawa::Edge::CALL){
				display::Edge *display_edge;
				display_edge = _graph->newEdge(node,map.get(edge->target()));
				onEdge(*edge, display_edge);
			}
			else {
				Node *cfg_node = _graph->newNode();
				onCall(edge->calledCFG(), cfg_node);
				display::Edge *display_edge = _graph->newEdge(node, cfg_node);
				onEdge(edge, display_edge);
			}
		}
	}
	onEnd(_graph);
	_made = true;
}


/**
 * This function only displays the graph made.
 */
void CFGDrawer::display(void){
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
void CFGDrawer::onInit(PropList& graph, PropList& nodes, PropList& edges){
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
void CFGDrawer::onNode(otawa::BasicBlock *bb, otawa::display::Node *node){
	SHAPE(node) = ShapeStyle::SHAPE_MRECORD;

	// make title
	StringBuffer title;
	title << bb; //->number() << " (0x" << fmt::address(bb->address()) << ')';
	TITLE(node) = title.toString();

	// make body
	StringBuffer body;
	for(BasicBlock::InstIter inst(bb); inst; inst++){
		if(body.length() > 0)
			body << '\n';

		// Display labels
		for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label; label++)
			body << *label << ":\n";
		for(Identifier<String>::Getter label(inst, otawa::LABEL); label; label++)
			body << *label << ":\n";


		/*if(inst->hasProp(FUNCTION_LABEL)){
			String label = FUNCTION_LABEL(inst);
			body << label << ":\n";
		}
		if(inst->hasProp(LABEL)){
			String label = LABEL(inst);
			body << label << ":\n";
		}*/

		body << "0x" << ot::address(inst->address()) << ":  ";
		inst->dump(body);
	}
	if(body.length() > 0)
		BODY(node) = body.toString();

	// give special format for Entry and Exit
	if(bb->isEntry()){
		TITLE(node) = "ENTRY";
	}
	else if(bb->isExit()){
		TITLE(node) = "EXIT";
	}
	else {
		node->setProps(*bb);
	}
}


/**
 * This function is called when a new Edge is created.
 * One can give properties to the edge
 * @param cfg_edge Edge of the CFG from which the Edge of the Graph has been made
 * @param display_edge Edge of the Graph made. One can give properties to it
 */
void CFGDrawer::onEdge(otawa::Edge *cfg_edge, otawa::display::Edge *display_edge){
	switch(cfg_edge->kind()){
		case otawa::Edge::CALL:
			LABEL(display_edge) = "call";
			break;
		case otawa::Edge::TAKEN:
			LABEL(display_edge) = "taken";
			break;
		case otawa::Edge::NOT_TAKEN:
			LABEL(display_edge) = "";
			break;
		case otawa::Edge::VIRTUAL:
			LABEL(display_edge) = "virtual";
			STYLE(display_edge) = STYLE_DASHED;
			break;
		case otawa::Edge::VIRTUAL_CALL:
			LABEL(display_edge) = "call";
			STYLE(display_edge) = STYLE_DASHED;
			break;
		case otawa::Edge::VIRTUAL_RETURN:
			LABEL(display_edge) = "return";
			STYLE(display_edge) = STYLE_DASHED;
			break;
		default:
			ASSERT(false);
	}
	display_edge->setProps(*cfg_edge);
}



/**
 * This function is called when the CFG have been drawn.
 * One can add nodes, edges, or properties to the graph.
 * @param graph graph being drawn
 */
void CFGDrawer::onEnd(otawa::display::Graph *graph){
}


/**
 * This function is called to display a node representing a called CFG.
 */
void CFGDrawer::onCall(CFG *cfg, display::Node *node) {
	TITLE(node) = cfg->label();
}

} }

