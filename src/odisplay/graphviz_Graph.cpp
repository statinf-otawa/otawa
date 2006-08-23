/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/graphviz_Graph.cpp -- GraphVizGraph class implementation.
 */
#include "graphviz.h"
#include <elm/io/UnixOutStream.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace elm::genstruct;
using namespace elm::io;

namespace otawa { namespace display {


/**
 * @author G. Cavaignac
 * @class GraphVizGraph
 * This class represents the graph that will be drawn by graphviz
 */


/**
 * Constructs a new GraphVizGraph
 * @param defaultGraphStyle default properties for the graph
 * Can be changed with the methods inherited from PropList
 * @param defaultNodeStyle default properties for the nodes
 * Can be changed with the methods inherited from PropList
 * @param defaultEdgeStyle default properties for the edges
 * Can be changed with the methods inherited from PropList
 */
GraphVizGraph::GraphVizGraph(
	const PropList& defaultGraphStyle,
	const PropList& defaultNodeStyle,
	const PropList& defaultEdgeStyle)
: _node_count(0)
{
	_default_node_style.addProps(defaultNodeStyle);
	_default_edge_style.addProps(defaultEdgeStyle);
	addProps(defaultGraphStyle);
}


void GraphVizGraph::printOthersAttributes(Output& out){
	String props = getPropertiesString();
	out << "label=\"" << quoteSpecials(props) << "\\l\"";
}


String GraphVizGraph::attributes(){
	return GraphVizItem::attributes(*this);
}


void GraphVizGraph::setProps(const PropList& props){
	GraphVizItem::setProps(props);
}



Node *GraphVizGraph::newNode(
	const PropList& style,
	const PropList& props)
{
	GraphVizNode *node = new GraphVizNode(_node_count++);
	node->addProps(_default_node_style);
	node->addProps(style);
	node->setProps(props);
	_nodes += node;
	return node;
}


Edge *GraphVizGraph::newEdge(
	Node *source,
	Node *target,
	const PropList& style,
	const PropList& props)
{
	assert(source);
	assert(target);
	GraphVizNode *src;
	GraphVizNode *dest;
	GraphVizEdge *edge;
	src = dynamic_cast<GraphVizNode*>(source);
	dest = dynamic_cast<GraphVizNode*>(target);
	assert(src);
	assert(dest);
	edge = new GraphVizEdge(src, dest);
	edge->addProps(_default_edge_style);
	edge->addProps(style);
	edge->setProps(props);
	_edges += edge;
	return edge;
}




/**
 * Prints to the given output all the data
 * DOT needs to create the graph information
 */
void GraphVizGraph::printGraphData(Output& out){
	out << "digraph main{\n";
	String attrs = attributes();
	if(!attrs.isEmpty())
		out << "\tgraph " << attrs << '\n';
	for(FragTable<GraphVizNode*>::Iterator iter(_nodes); !iter.ended(); iter.next()){
		GraphVizNode *node;
		node = iter.item();
		out << "\tNode" << node->number();
		out << ' ' << node->attributes() << ";\n";
	}
	for(FragTable<GraphVizEdge*>::Iterator iter(_edges); !iter.ended(); iter.next()){
		GraphVizEdge *edge;
		GraphVizNode *src;
		GraphVizNode *dest;
		edge = iter.item();
		src = edge->source();
		dest = edge->target();
		out << "\tNode" << src->number() << " -> Node" << dest->number();
		out << ' ' << edge->attributes() << ";\n";
	}
	out << "}\n";
}


void GraphVizGraph::display(void){
	CString command;
	CString param_type;
	CString file = GRAPHVIZ_FILE(this);;
	switch(GRAPHVIZ_LAYOUT(this)){
		case LAYOUT_DOT:
			command = "dot";
			break;
		case LAYOUT_RADIAL:
			command = "twopi";
			break;
		case LAYOUT_CIRCULAR:
			command = "circo";
			break;
		case LAYOUT_UNDIRECTED_NEATO:
			command = "neato";
			break;
		case LAYOUT_UNDIRECTED_FDP:
			command = "fdp";
			break;
		default:
			assert(false);
	}
	switch(GRAPHVIZ_OUTPUT(this)){
		case OUTPUT_TEXT:
			param_type = "-Tdot";
			break;
		case OUTPUT_PS:
			param_type = "-Tps";
			break;
		case OUTPUT_PNG:
			param_type = "-Tpng";
			break;
		case OUTPUT_SVG:
			param_type = "-Tsvg";
			break;
		default:
			assert(false);
	}
	
	int pipe_graph_dot[2];
	if(pipe(pipe_graph_dot) == -1){
		cerr << "Error while creating pipe\n";
		return;
	}
	/*int file_descr = open(&file, O_CREAT, 0644);
	if(file_descr < 0){
		cerr << "Error while creating the file " << file << endl;
		return;
	}
	close(file_descr);*/
	pid_t child;
	switch(child = fork()){
		case -1:
			cerr << "Error while creating new process\n";
			return;
		case 0:
			{
				close(pipe_graph_dot[1]);
				dup2(pipe_graph_dot[0], 0);
				execlp(&command, &command, &param_type, "-o" , &file, NULL);
				exit(0);
			}
		default:
			{
				close(pipe_graph_dot[0]);
				UnixOutStream unixOutStream(pipe_graph_dot[1]);
				Output output(unixOutStream);
				printGraphData(output);
				close(unixOutStream.fd());
				close(pipe_graph_dot[1]);
				waitpid(child, 0, 0);
			}
	}
}



} }






