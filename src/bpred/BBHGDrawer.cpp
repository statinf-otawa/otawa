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
#include <otawa/display/Displayer.h>
#include <elm/data/HashMap.h>


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
BBHGDrawer::BBHGDrawer(BBHG *bhg, sys::Path path): _made(false), _path(path) {
	_bhg = bhg;
}


/**
 * This function only displays the graph made.
 */
void BBHGDrawer::display(void){
	display::Displayer *disp = display::Provider::display(_bhg, *this);
	disp->setPath(_path);
	onInit(disp->defaultVertex(), disp->defaultEdge());
	disp->process();
}


/**
 * This function is called before creating the Graph.
 * One can give some properties to the PropLists
 * @param general PropList for the default behaviour and properties of the graph
 * @param nodes PropList for the default properties of nodes
 * @param edges PropList for the default properties of edges
 */
void BBHGDrawer::onInit(display::VertexStyle& vertex_style, display::EdgeStyle& edge_style) const {
	vertex_style.shape = display::ShapeStyle::SHAPE_MRECORD;
	vertex_style.text.size = 12;
	edge_style.text.size = 12;
}


/**
 * This function is called when a new Node is created.
 * One can give properties to the node
 * @param bb BasicBlock from which the node has been made
 * @param node Node made. One can give some properties to it
 */
void BBHGDrawer::onNode(BBHGNode *bb, display::Text& caption, display::VertexStyle& style) const {
	style.shape = display::ShapeStyle::SHAPE_MRECORD;
	caption << display::begin(display::TABLE);

	// make title
	caption << display::begin(display::ROW) << display::begin(display::CELL);
	caption << bb->getCorrespondingBB()->index() << ":";
	for(int i=bb->getHistory().size()-1;i>=0;--i)
		caption << ((bb->getHistory().contains(i))?"1":"0");
	caption << display::end(display::CELL) << display::end(display::ROW) << display::hr;

	// make body
	caption << display::begin(display::ROW) << display::begin(display::CELL);
	if( bb->isEntry() || bb->isExit() ) {
		if(bb->isEntry())
			caption << "ENTRY ";
		if(bb->isExit()) {
			caption << "EXIT :";
			if(bb->exitsWithNT())
				caption << "NT ";
			if(bb->exitsWithT())
				caption << "T";
		}
	}
	caption << display::end(display::CELL) << display::end(display::ROW);

	caption << display::end(display::TABLE);
}


/**
 * This function is called when a new Edge is created.
 * One can give properties to the edge
 * @param bhg_edge Edge of the BBHG from which the Edge of the Graph has been made
 * @param display_edge Edge of the Graph made. One can give properties to it
 */
void BBHGDrawer::onEdge(BBHGEdge *bhg_edge, display::Text& label, display::EdgeStyle& style) const {

	if(bhg_edge->isTaken()) {
		label << "T";
	}
	else if(!bhg_edge->isFromBranch()){
		label << "";
	}
	else {
		label << "NT";
	}

}


/**
 * This function is called to display a node representing a called BBHG.
 */
void BBHGDrawer::onCall(BBHG *bhg, display::Text& caption, display::GraphStyle& style) const {
	caption << "classe @" << bhg->getClass();
}


/**
 */
void BBHGDrawer::decorate(graph::DiGraph *graph, display::Text& caption, display::GraphStyle& style) const {
	onCall(_bhg, caption, style);
}


/**
 */
void BBHGDrawer::decorate(graph::DiGraph *graph, graph::Vertex *vertex, display::Text& content, display::VertexStyle& style) const {
	onNode(static_cast<BBHGNode *>(vertex), content, style);
}


/**
 */
void BBHGDrawer::decorate(graph::DiGraph *graph, graph::Edge *edge, display::Text& label, display::EdgeStyle& style) const {
	onEdge(static_cast<BBHGEdge *>(edge), label, style);
}

} }		// otawa::bpred

