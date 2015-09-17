/*
 *	$Id$
 *	Graph class interface
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
#ifndef OTAWA_GRAPH_GRAPH_H
#define OTAWA_GRAPH_GRAPH_H

#include <elm/assert.h>
#include <elm/PreIterator.h>
#include <elm/genstruct/FragTable.h>
#include <elm/util/BitVector.h>
#include <elm/genstruct/VectorQueue.h>

namespace otawa { namespace graph {

// Predefinition
class Node;
class Edge;
class Graph;


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
	inline Graph *graph(void) const { return _graph; }
	inline int index(void) const { return idx; }
	inline bool hasSucc(void) const { return outs; }
	inline bool hasPred(void) const { return ins; }
	inline int countSucc(void) const {
		int cnt = 0;
		for(Successor edge(this); edge; edge++)
			cnt++;
		return cnt;
	}
	inline int countPred(void) const {
		int cnt = 0;
		for(Predecessor edge(this); edge; edge++)
			cnt++;
		return cnt;
	}
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
	inline Edge(Node *source, Node *target): src(source), tgt(target) {
		ASSERT(source->graph() == target->graph());
		sedges = src->outs;
		src->outs = this;
		tedges = tgt->ins;
		tgt->ins = this;
	}

	// Accessors
	inline Node *source(void) const  { return src; }
	inline Node *target(void) const  { return tgt; }
};


// Graph class
class Graph {
	friend class Node;
	friend class Edge;
public:
	typedef otawa::graph::Node *Vertex;
	typedef otawa::graph::Edge *Edge;
	~Graph(void);
	void remove(graph::Edge *edge);

	// Collection concept
	inline int count(void) const { return nodes.count(); }
	inline bool contains(Node *item) const { return nodes.contains(item); }
	inline bool isEmpty(void) const { return nodes.isEmpty(); }
 	inline operator bool(void) const { return !isEmpty(); }
	
	// Iterator class
	class Iterator: public elm::genstruct::FragTable<Node *>::Iterator {
	public:
		inline Iterator(const Graph *graph)
			: elm::genstruct::FragTable<Node *>::Iterator(graph->nodes) { }
		inline Iterator(const Iterator& iter)
			: elm::genstruct::FragTable<Node *>::Iterator(iter) { }
	};

	// MutableCollection concept
	void clear(void);
	void add(Node *node);
	template <template <class _> class C> void addAll(const C<Node *> &items)
		{ for(typename C<Node *>::Iterator item(items); item; item++) add(item); }
	void remove(Node *node);
	template <template <class _> class C> void removeAll(const C<Node *> &items)
		{ for(typename C<Node *>::Iterator item(items); item; item++) remove(item); }
	inline void remove(const Iterator& iter) { nodes.remove(iter); }
	
	// DiGraph concept
	Node *sinkOf(graph::Edge *edge) const { return edge->target(); }
	int outDegree(Node *vertex) const;
	bool isSuccessorOf(Node *succ, Node *ref) const;

	// OutIterator class
	class OutIterator: public elm::PreIterator<OutIterator, graph::Edge *> {
	public:
		inline OutIterator(const Node *source): iter(source) { }
		inline OutIterator(const Graph& graph, const Node *source): iter(source) { }
		inline OutIterator(const OutIterator& iterator): iter(iterator.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline graph::Edge *item(void) const { return iter.edge(); }
		inline void next(void) { iter.next(); }
	private:
		Node::Successor iter;
	};

	// BiDiGraph concept
	Node *sourceOf(graph::Edge *edge) const { return edge->source(); }
	int inDegree(Node *vertex) const;
	bool isPredecessorOf(Node *prev, Node *ref) const;

	// InIterator class
	class InIterator: public elm::PreIterator<InIterator, graph::Edge *> {
	public:
		inline InIterator(const Node *source): iter(source) { }
		inline InIterator(const Graph& graph, const Node *source): iter(source) { }
		inline InIterator(const InIterator& iterator): iter(iterator.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline graph::Edge *item(void) const { return iter.edge(); }
		inline void next(void) { iter.next(); }
	private:
		Node::Predecessor iter;
	};
	
	// DiGraphWithIndexedVertex concept
	inline int indexOf(Node *vertex) const { return vertex->index(); }

private:
	elm::genstruct::FragTable<Node *> nodes;
};


// Node Inlines
inline Node::Node(Graph *graph)
: _graph(0), idx(-1), ins(0), outs(0) {
	if(graph)
		graph->add(this);
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

// Node::Successor inlines
inline Node::Successor::Successor(const Node *node): _edge(node->outs) {
	ASSERT(node);
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
	ASSERT(node);
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

