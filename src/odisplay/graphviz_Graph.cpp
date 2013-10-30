/*
 *	$Id$
 *	graphviz plugin implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-07, IRIT UPS.
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

#include "graphviz.h"
#include <otawa/display/Driver.h>
#include <elm/io/UnixOutStream.h>
#include <elm/system/ProcessBuilder.h>
#include <elm/system/System.h>

using namespace elm::genstruct;
using namespace elm::io;
using namespace elm::system;

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
	//String props = getPropertiesString();
	string label = BODY(this);
	if(label)
		out << "label=\"" << quoteSpecials(label) << "\\l\"";
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
	ASSERT(source);
	ASSERT(target);
	GraphVizNode *src;
	GraphVizNode *dest;
	GraphVizEdge *edge;
	src = dynamic_cast<GraphVizNode*>(source);
	dest = dynamic_cast<GraphVizNode*>(target);
	ASSERT(src);
	ASSERT(dest);
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


void GraphVizGraph::display(void) throw(DisplayException) {
	String file = display::OUTPUT_PATH(this);
	
	// case of RAW dot
	if(OUTPUT_KIND(this) == OUTPUT_RAW_DOT) {
		io::OutStream *out_stream = &io::out;
		if(file)
			try {
				out_stream = sys::System::createFile(file);
			}
			catch(sys::SystemException& e) {
				throw DisplayException(_ << " creating \"" << file << "\": " << e.message());
			}
		io::Output output(*out_stream);
		printGraphData(output);
		if(file)
			delete out_stream;
		return;
	}

	// Select the command
	CString command;	
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
			throw DisplayException("unsupported layout");
	}
	elm::system::ProcessBuilder builder(command);

	// Select the type
	CString param_type;
	switch(OUTPUT_KIND(this)){
		case OUTPUT_DOT:
			param_type = "-Tdot";
			break;
		case OUTPUT_PDF:
		case OUTPUT_PS:
		case OUTPUT_ANY:
			param_type = "-Tps";
			break;
		case OUTPUT_PNG:
			param_type = "-Tpng";
			break;
		case OUTPUT_GIF:
			param_type = "-Tgif";
			break;
		case OUTPUT_JPG:
			param_type = "-Tjpg";
			break;
		case OUTPUT_SVG:
			param_type = "-Tsvg";
			break;
		default:
			throw DisplayException("unsupported output kind");
	}
	builder.addArgument(param_type);
	
	// Add the output
	if(file) {
		builder.addArgument("-o");
		builder.addArgument(&file);
	}
	
	// Redirect IO
	Pair<PipeInStream *, PipeOutStream *> pipe = System::pipe();
	builder.setInput(pipe.fst);
	Process *proc = builder.run();
	
	// Perform the output
	Output output(*pipe.snd);
	printGraphData(output);

	// Cleanup
	delete pipe.fst;
	delete pipe.snd;
	proc->wait();
	if(proc->returnCode())
		throw DisplayException(_ << "failure of dot call: " << proc->returnCode());
	delete proc;
}

} }	// otawa::display






