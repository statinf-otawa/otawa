/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 * 
 * prog/util_Graph.cpp -- Graph class implementation.
 */

#include <otawa/util/Graph.h>
#include <elm/io.h>

using namespace elm;
using namespace elm::genstruct;

namespace otawa { namespace graph {


/**
 * @class Graph
 * This class represents a full graph with nodes and edges.
 * It is not usually used as is : it may be embedded in some other object
 * representing a graph and the Node and Edge classes is redefined to be valued
 * according the requirement of the represented graph.
 */


/**
 */
Graph::~Graph(void) {
	for(NodeIterator node(this); node; node++) {
		for(Edge *edge = node->outs, *next; edge; edge = next) {
			next = edge->sedges;
			delete edge;
		}
		node->_graph = 0;
		delete *node;
	}
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
	assert(node->graph() == this);
	
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
 * Remove and destroy a node from the graph.
 * @param node	Node to destroy.
 */
void Graph::destroy(Node * node) {
	remove(node);
	delete node;
}


/**
 * Destroy an edge.
 * @param edge	Edge to destroy.
 */
void Graph::destroy(Edge *edge) {
	
	// Remove edge from successor list
	Edge *prev = 0;
	for(Edge *cur = edge->src->outs; cur != edge; prev = cur, cur = cur->sedges)
		assert(cur);
	if(prev)
		prev->sedges = edge->sedges;
	else
		edge->src->outs = edge->sedges;
		
	// Remove edge from predecessor list
	prev = 0;
	for(Edge *cur = edge->tgt->ins; cur != edge; prev = cur, cur = cur->tedges)
		assert(cur);
	if(prev)
		prev->tedges = edge->tedges;
	else
		edge->tgt->ins = edge->tedges;
	
	// Delete it finally
	delete edge;
}


/**
 * @class Graph::NodeIterator
 * A simple iterator on the nodes contained in a graph.
 */


/**
 * @class Graph::NodeIterator::NodeIterator(const Graph& graph);
 * Build an iterator on the given graph.
 */


/**
 * @class Graph::PreorderIterator
 * An iterator allowing to traverse the graph using preorder, that is, a
 * node is only traversed when its predecessors has been traversed.
 * @warning Be careful. A cycle in the graph may induce infinite loop.
 */


/**
 * Build a preorder iterator.
 * @param graph	Graph to traverse.
 * @param entry	Entry of the graph.
 */
Graph::PreorderIterator::PreorderIterator(const Graph *graph, Node *entry)
: _graph(graph), visited(_graph->nodes.length()) {
	queue.put(entry);
}


/**
 */
bool Graph::PreorderIterator::ended(void) const {
	return queue.isEmpty();
}


/**
 */
Node *Graph::PreorderIterator::item(void) const {
	return queue.head();
}


/**
 */
void Graph::PreorderIterator::next(void) {
	Node *node = queue.get();
	visited.set(node->index());
	for(Node::Successor succ(node); succ; succ++)
		if(succ != node) {
			assert(!visited.bit(succ->index()));
			bool check = true;
			for(Node::Predecessor pred(succ); pred; pred++) {
				check = visited.bit(pred->index());
				if(!check)
					break;
			}
			if(check)
				queue.put(succ);
		}
}

} } // otawa::graph
