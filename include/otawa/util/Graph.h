/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 * 
 * otawa/util/Graph.h -- Graph, Node, Edge classes interfaces.
 */
#ifndef OTAWA_UTIL_GRAPH_H
#define OTAWA_UTIL_GRAPH_H

#include <assert.h>
#include <elm/Iterator.h>
#include <elm/genstruct/Vector.h>
#include <elm/util/BitVector.h>
#include <elm/genstruct/VectorQueue.h>

namespace otawa { namespace graph {

// Predefinition
class Node;
class Edge;
class Graph;


// Graph class
class Graph {
	friend class Node;
	friend class Edge;
	elm::genstruct::Vector<Node *> nodes;
public:
	~Graph(void);
	
	// Mutator
	void add(Node * node);
	void remove(Node *node);
	void destroy(Node * node);
	void destroy(Edge *edge);
	
	// NodeIterator class
	class NodeIterator: public elm::genstruct::Vector<Node *>::Iterator {
	public:
		inline NodeIterator(const Graph *graph);
		inline NodeIterator(const NodeIterator& iter);
	};
	
	// PreorderIterator class
	class PreorderIterator: public elm::PreIterator<PreorderIterator, Node *> {
		const Graph *_graph;
		elm::BitVector visited, queued;
		elm::genstruct::VectorQueue<Node *> queue;
	public:
		PreorderIterator(const Graph *graph, Node *entry);
		bool ended(void) const;
		Node *item(void) const;
		void next(void);
	};
};


// Node class	
class Node {
	friend class Graph;
	friend class Edge;
	Graph *_graph;
	int idx;
	Edge *ins, *outs;
	void unlink(void);
protected:
	inline Node(Graph *graph = 0);
	virtual ~Node(void);
public:
	
	// Successor class
	class Successor: public elm::PreIterator<Successor, Node *> {
		Edge *_edge;
	public:
		inline Successor(const Node *node);
		inline bool ended(void) const;
		inline void next(void);
		inline Node *item(void) const;
		inline Edge *edge(void) const;
	};
	
	// Predecessor class
	class Predecessor: public elm::PreIterator<Predecessor, Node *> {
		Edge *_edge;
	public:
		inline Predecessor(const Node *node);
		inline bool ended(void) const;
		inline void next(void);
		inline Node *item(void) const;
		inline Edge *edge(void) const;
	};

	// Accessors
	inline Graph *graph(void) const;	
	inline int index(void) const;
	inline bool hasSucc(void) const;
	inline bool hasPred(void) const;
	inline int countSucc(void) const;	
	inline int countPred(void) const;
	inline bool isPredOf(const Node *node);
	inline bool isSuccOf(const Node *node);
};


// Edge class
class Edge {
	friend class Node;
	friend class Graph;
	friend class Node::Successor;
	friend class Node::Predecessor;
	Node *src, *tgt;
	Edge *sedges, *tedges;
protected:
	virtual ~Edge(void);
public:
	inline Edge(Node *source, Node *target);

	// Accessors
	inline Node *source(void) const;
	inline Node *target(void) const;
};


// Graph::NodeIterator class
inline Graph::NodeIterator::NodeIterator(const Graph *graph):
elm::genstruct::Vector<Node *>::Iterator(graph->nodes) {
}

inline Graph::NodeIterator::NodeIterator(const NodeIterator& iter)
: elm::genstruct::Vector<Node *>::Iterator(iter) {
}


// Edge inlines
inline Edge::Edge(Node *source, Node *target): src(source), tgt(target) {
	assert(source->graph() == target->graph());
	sedges = src->outs;
	src->outs = this;
	tedges = tgt->ins;
	tgt->ins = this;
}

inline Node *Edge::source(void) const {
	return src;
}

inline Node *Edge::target(void) const {
	return tgt;
}


// Node Inlines
inline Node::Node(Graph *graph)
: _graph(0), idx(-1), ins(0), outs(0) {
	if(graph)
		graph->add(this);
}

inline Graph *Node::graph(void) const {
	return _graph;
}

inline int Node::index(void) const {
	return idx;
}

inline bool Node::hasSucc(void) const {
	return outs;
}

inline bool Node::hasPred(void) const {
	return ins;
}

inline bool Node::isPredOf(const Node *node) {
	for(Successor succ(this); succ; succ++)
		if(succ == node)
			return true;
	return false;
}
	
inline bool Node::isSuccOf(const Node *node) {
	for(Predecessor pred(this); pred; pred++)
		if(pred == node)
			return true;
	return false;
}

inline int Node::countSucc(void) const {
	int cnt = 0;
	for(Successor edge(this); edge; edge++)
		cnt++;
	return cnt;
}
	
inline int Node::countPred(void) const {
	int cnt = 0;
	for(Predecessor edge(this); edge; edge++)
		cnt++;
	return cnt;
}
	
// Node::Successor inlines
inline Node::Successor::Successor(const Node *node): _edge(node->outs) {
	assert(node);
}

inline bool Node::Successor::ended(void) const {
	return !_edge;
}

inline Node *Node::Successor::item(void) const {
	return _edge->target();
}

inline void Node::Successor::next(void) {
	_edge = _edge->sedges;
}

inline Edge *Node::Successor::edge(void) const {
	return _edge;
}


// Node::Predecessor inlines
inline void Node::Predecessor::next(void) {
	_edge = _edge->tedges;
}

inline Node::Predecessor::Predecessor(const Node *node): _edge(node->ins) {
	assert(node);
}

inline bool Node::Predecessor::ended(void) const {
	return !_edge;
}

inline Node *Node::Predecessor::item(void) const {
	return _edge->source();
}

inline Edge *Node::Predecessor::edge(void) const {
	return _edge;
}

} } // otawa::graph

#endif // OTAWA_UTIL_GRAPH_H

