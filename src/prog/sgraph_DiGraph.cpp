/*
 *	DiGraph class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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

#include "../../include/otawa/graph/DiGraph.h"

namespace otawa { namespace graph {

/**
 * @defgroup graph Simple Graph
 *
 * This group provides classes to represent simple directed-graph that may be
 * traversed in a bidirectionnal ways and algorithms to work on these graphs.
 * To make easier these algorithms, this type of graph assign to each vertex
 * a unique index.
 */

/**
 * @class Edge
 * Represents an edge between two vertices in a @ref sgraph .
 * @ingroup graph
 */

/**
 */
Edge::~Edge(void) {
	src->_outs.remove(this);
	snk->_ins.remove(this);
}

/**
 * @fn Vertex *Edge::source(void) const;
 * Get the source vertex.
 * @return	Source vertex.
 */

/**
 * @fn Vertex *Edge::sink(void) const;
 * Get the sink vertex.
 * @return	Sink vertex.
 */


/**
 * @class Vertex
 * Represents a vertex in the @ref sgraph graphs.
 * @ingroup graph
 */

/**
 */
Vertex::~Vertex(void) {
}


/**
 * @fn int Vertex::index(void) const;
 * Get the index number of the vertex.
 * @return	Vertex index.
 */


/**
 * @fn EdgeIter Vertex::ins(void) const;
 * Get a iterator on the list of entering edges on the current vertex.
 * @return	Iterator on entering edges.
 */


/**
 * @fn EdgeIter Vertex::outs(void) const;
 * Get a iterator on the list of leaving edges on the current vertex.
 * @return	Iterator on leaving edges.
 */


/**
 * @fn EdgeIter Vertex::countIns(void) const;
 * Get the amount of entering edges on the current vertex.
 * @return	Iterator on entering edges.
 */


/**
 * @fn EdgeIter Vertex::countOuts(void) const;
 * Get the amount of leaving edges on the current vertex.
 * @return	Iterator on leaving edges.
 */


/**
 * @class DiGraph
 * Directed graph representation in the @ref sgraph group.
 * @ingroup graph
 */

/**
 * @fn Vertex *DiGraph::entry(void) const;
 * Get the entry vertex of the graph.
 * @return	Entry vertex.
 */

/**
 * @fn VertexIter DiGraph::vertices(void) const;
 * Get an iterator on vertices of the graph.
 * @return	Iterator on graph vertices.
 */

/**
 * @fn int DiGraph::count(void) const;
 * Get the count of vertices in the graph.
 * @return	Vertices count.
 */


/**
 * @class DiGraphBuilder
 * Builder for a DiGraph.
 *
 * @ingroup graph
 */

/**
 * Construct the DiGraph builder.
 * @param g	Real digraph.
 * @param v	Entry vertex.
 */
DiGraphBuilder::DiGraphBuilder(DiGraph *g): _g(g), c(0) {
	ASSERT(_g);
}


/**
 * Construct the DiGraph builder.
 * @param v	Entry vertex.
 */
DiGraphBuilder::DiGraphBuilder(void): _g(new DiGraph()), c(0) {
}


/**
 * @fn Vertex *DiGraphBuilder::entry(void) const;
 * Get the entry vertex.
 * @return	Entry vertex.
 */


/**
 * Add a vertex to the graph.
 * @param v	Vertex to add.
 */
void DiGraphBuilder::add(Vertex *v) {
	ASSERTP(_g, "graph is not yet built!");
	v->idx = c++;
	_g->v.add(v);
}

/**
 * Add a vertex to the graph.
 * @param v	Vertex to add.
 */
void DiGraphBuilder::remove(Vertex *v) {
	ASSERTP(_g, "graph is not yet built!");
	for(DiGraph::VertexIter iter = _g->begin() ; iter != _g->end() ; iter++) {
		Vertex* vv = *iter;
		if(vv->idx > v->idx)
			--vv->idx;
	}
	--c;
	_g->v.remove(v);
}



/**
 * Add an edge.
 * @param v		Source vertex.
 * @param w		Sink vertex.
 * @param e		Edge.
 * @param off	If set to true (default false), the edge is not recorded
 * 				in sink and source edge.
 */
void DiGraphBuilder::add(Vertex *v, Vertex *w, Edge *e, bool off) {
	ASSERTP(_g, "graph is not yet built!");
	e->src = v;
	e->snk = w;
	if(!off) {
		v->_outs.add(e);
		w->_ins.add(e);
	}
}


/**
 * Build the graph and return it. After this call, no more method
 * of this instance can be called.
 * @return	Built graph.
 */
DiGraph *DiGraphBuilder::build(void) {
	ASSERTP(_g, "graph is not yet built!");
	DiGraph *r = _g;
	_g = 0;
	return r;
}

} }		// otawa::sgraph
