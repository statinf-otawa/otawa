/*
 *	$Id$
 *	Graph class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <otawa/util/Graph.h>
#include <elm/io.h>

using namespace elm;
using namespace elm::genstruct;

namespace otawa { namespace graph {


/**
 * @defgroup graph Graph Handling
 * 
 * OTAWA provides several graph implementations and algorithm to use them.
 * A graph is OTAWA-compliant if it implements the @ref otawa::concept::DiGraph
 * concept and any provided algorithm may be applied to it.
 * 
 * The graph implements includes:
 * @li @ref otawa::graph::Graph,
 * @li @ref otawa::graph::GenGraph.
 * 
 * The following algorithm are provided:
 * @li @ref otawa::graph::PreorderIterator.
 */


/**
 * @class Graph
 * This class represents a full graph with nodes and edges.
 * It is not usually used as is : it may be embedded in some other object
 * representing a graph and the Node and Edge classes is redefined to be valued
 * according the requirement of the represented graph.
 * 
 * @par Implemented concepts
 * @li @ref otawa::concept::DiGraph
 * @li @ref otawa::concept::BiDiGraph
 * @li @ref otawa::concept::DiGraphWithIndexedVertex
 * 
 * @ingroup graph
 */


/**
 */
void Graph::clear(void) {
	for(Iterator node(this); node; node++) {
		for(graph::Edge *edge = node->outs, *next; edge; edge = next) {
			next = edge->sedges;
			delete edge;
		}
		node->_graph = 0;
		delete *node;
	}
	nodes.clear();
}


/**
 */
Graph::~Graph(void) {
	clear();
}


/**
 * Add new node.
 * @param node	Node to add.
 */
void Graph::add(Node *node) {
	node->_graph = this;
	node->idx = nodes.length();
	nodes.add(node);
}


/**
 * Remove a node from a graph.
 * @param node	Node to remove.
 */
void Graph::remove(Node *node) {
	ASSERT(node->graph() == this);
	
	// Remove from the vector
	nodes.removeAt(node->idx);
	for(int i = node->idx; i < nodes.length(); i++)
		nodes[i]->idx--;
	
	// Remove from the links
	node->unlink();
	node->_graph = 0;
	node->idx = -1;
}


/**
 * Destroy an edge.
 * @param edge	Edge to destroy.
 */
void Graph::remove(graph::Edge *edge) {
	
	// Remove edge from successor list
	graph::Edge *prev = 0;
	for(graph::Edge *cur = edge->src->outs; cur != edge; prev = cur, cur = cur->sedges)
		ASSERT(cur);
	if(prev)
		prev->sedges = edge->sedges;
	else
		edge->src->outs = edge->sedges;
		
	// Remove edge from predecessor list
	prev = 0;
	for(graph::Edge *cur = edge->tgt->ins; cur != edge; prev = cur, cur = cur->tedges)
		ASSERT(cur);
	if(prev)
		prev->tedges = edge->tedges;
	else
		edge->tgt->ins = edge->tedges;
	
	// Delete it finally
	delete edge;
}


/**
 * Get the out degree of the given vertex.
 * @param vertex	Vertex to compute out degree for.
 * @return			Out degree of the vertex.
 */
int Graph::outDegree(Node *vertex) const {
	int cnt = 0;
	for(OutIterator edge(*this, vertex); edge; edge++)
		cnt++;
	return cnt;
}


/**
 * Test if the vertex succ is successor of the vertex ref.
 * @param succ	Successor vertex.
 * @param ref	Reference vertex.
 * @return		True if succ is successor, false else.
 */ 
bool Graph::isSuccessorOf(Node *succ, Node *ref) const {
	for(OutIterator edge(*this, ref); edge; edge++)
		if(sinkOf(edge) == succ)
			return true;
	return false;
}


/**
 * Get the in degree of the given vertex.
 * @param vertex	Vertex to compute out degree for.
 * @return			Out degree of the vertex.
 */
int Graph::inDegree(Node *vertex) const {
	int cnt = 0;
	for(InIterator edge(*this, vertex); edge; edge++)
		cnt++;
	return cnt;
}


/**
 * Test if the vertex pred is predecessor of the vertex ref.
 * @param pred	Predecessor vertex.
 * @param ref	Reference vertex.
 * @return		True if pred is predecessor, false else.
 */ 
bool Graph::isPredecessorOf(Node *pred, Node *ref) const {
	for(OutIterator edge(*this, ref); edge; edge++)
		if(sourceOf(edge) == pred)
			return true;
	return false;
}


/**
 * @class Graph::Iterator
 * A simple iterator on the nodes contained in a graph.
 */


/**
 * @fn Graph::Iterator::Iterator(const Graph& graph);
 * Build an iterator on the given graph.
 */

} } // otawa::graph
