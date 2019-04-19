/*
 *	graph module implementation and documentation
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

#include "../../include/otawa/ograph/GenGraph.h"
#include "../../include/otawa/ograph/Graph.h"
#include "../../include/otawa/ograph/PreorderIterator.h"

namespace otawa { namespace ograph {

/**
 * @defgroup ograph Old Graph Management
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
 * @class GenGraph
 *
 * GenGraph is a subclass of @ref graph::Graph and provides a simple way to
 * customize nodes and edges.
 *
 * To use GenGraph, it is convenient to declare the following typedef depending
 * on your custom predeclared Node and Edge classes:
 * @code
 * class Node;
 * class Edge;
 * typedef GenGraph<Node, Edge> graph_t;
 * @endcode
 *
 * Then, you can declare your custom Node class (where you can store what you
 * want):
 * @code
 * class Node: public graph_t::GenNode {
 * public:
 *	...
 * };
 * @endcode
 *
 * The Edge a bit more tricky: you have to provide both ends of the edge
 * to the constructor as below:
 * @code
 * class Edge: public graph_t::GenEdge {
 * public:
 *	Edge(Node *src, Node *snk, ...): graph_t::GenEdge(src, snk) { }
 *	inline effect_t effect(void) const { return _eff; }
 *	...
 * };
 * @endcode
 *
 * For finish, you have just to declare the Graph itself:
 * @code
 * class Graph: public graph_t {
 * public:
 * 	...
 * };
 * @endcode
 *
 * The obtained class implements a bidirectional graph with all facilities
 * provided for @ref Graph available.
 *
 * @ingroup ograph
 */

/**
 * @class GenGraph<N, E> <otawa/util/GenGraph.h>
 * GenGraph is inherited from @ref Graph class but enforce the type checking of
 * the objects of graph: only nodes of type N and edges of type E are
 * accepted.
 * @param N	Type of nodes (must inherit from @ref GenGraph<N, E>::Node).
 * @param E Type of edges (must inherit from @ref GenGraph<N, E>::Edge).
 * @ingroup ograph
 */


/**
 * @class PreorderIterator
 * An iterator allowing to traverse the graph using preorder, that is, a
 * node is only traversed when its predecessors has been traversed.
 * @param G	Type of traversed graph.
 * @ingroup ograph
 */


/**
 * PreorderIterator::PreorderIterator(const G *graph, typename G::Vertex *entry);
 * Build a preorder iterator.
 * @param graph	Graph to traverse.
 * @param entry	Entry of the graph.
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
	for(Iter node(this); node(); node++) {
		for(ograph::Edge *edge = node->outs, *next; edge; edge = next) {
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
void Graph::remove(ograph::Edge *edge) {

	// Remove edge from successor list
	ograph::Edge *prev = 0;
	for(ograph::Edge *cur = edge->src->outs; cur != edge; prev = cur, cur = cur->sedges)
		ASSERT(cur);
	if(prev)
		prev->sedges = edge->sedges;
	else
		edge->src->outs = edge->sedges;

	// Remove edge from predecessor list
	prev = 0;
	for(ograph::Edge *cur = edge->tgt->ins; cur != edge; prev = cur, cur = cur->tedges)
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
	for(OutIterator edge(*this, vertex); edge(); edge++)
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
	for(OutIterator edge(*this, ref); edge(); edge++)
		if(sinkOf(*edge) == succ)
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
	for(InIterator edge(*this, vertex); edge(); edge++)
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
	for(OutIterator edge(*this, ref); edge(); edge++)
		if(sourceOf(*edge) == pred)
			return true;
	return false;
}


/**
 * @class Graph::Iter
 * A simple iterator on the nodes contained in a graph.
 */


/**
 * @fn Graph::Iterator::Iterator(const Graph& graph);
 * Build an iterator on the given graph.
 */

} } // otawa::ograph
