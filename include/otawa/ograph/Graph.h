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
#include <elm/data/Array.h>
#include <elm/PreIterator.h>
#include <elm/data/Array.h>
#include <elm/data/FragTable.h>
#include <elm/data/VectorQueue.h>
#include <elm/util/Option.h>
#include <elm/util/BitVector.h>

namespace otawa { namespace ograph {

using namespace elm;

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
		for(Successor edge(this); edge(); edge++)
			cnt++;
		return cnt;
	}
	inline int countPred(void) const {
		int cnt = 0;
		for(Predecessor edge(this); edge(); edge++)
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
	inline Edge(Node *source, Node *sink): src(source), tgt(sink) {
		ASSERT(source->graph() == sink->graph());
		sedges = src->outs;
		src->outs = this;
		tedges = tgt->ins;
		tgt->ins = this;
	}

	// Accessors
	inline Node *source(void) const  { return src; }
	inline Node *sink(void) const  { return tgt; }

	// deprecated
	inline Node *target(void) const  { return tgt; }
};


// Graph class
class Graph {
	friend class Node;
	friend class Edge;
public:
	typedef otawa::ograph::Node *Vertex;
	typedef otawa::ograph::Edge *Edge;
	~Graph(void);
	void remove(ograph::Edge *edge);

	// Collection concept
	inline int count(void) const { return nodes.count(); }
	inline bool contains(Node *item) const { return nodes.contains(item); }
	inline bool isEmpty(void) const { return nodes.isEmpty(); }
 	inline operator bool(void) const { return !isEmpty(); }
 	inline Node *at(int i) const { return nodes[i]; }
	
	// Iterator class
	class Iter: public elm::FragTable<Node *>::Iter {
	public:
		inline Iter(const Graph *graph): elm::FragTable<Node *>::Iter(graph->nodes) { }
	};

	// MutableCollection concept
	void clear(void);
	void add(Node *node);
	template <template <class _> class C> void addAll(const C<Node *> &items)
		{ for(typename C<Node *>::Iterator item(items); item; item++) add(item); }
	void remove(Node *node);
	template <template <class _> class C> void removeAll(const C<Node *> &items)
		{ for(typename C<Node *>::Iterator item(items); item; item++) remove(item); }
	inline void remove(const Iter& iter) { nodes.remove(iter); }
	
	// DiGraph concept
	Node *sinkOf(ograph::Edge *edge) const { return edge->sink(); }
	int outDegree(Node *vertex) const;
	bool isSuccessorOf(Node *succ, Node *ref) const;

	// OutIterator class
	class OutIterator: public elm::PreIterator<OutIterator, ograph::Edge *> {
	public:
		inline OutIterator(const Node *source): iter(source) { }
		inline OutIterator(const Graph& graph, const Node *source): iter(source) { }
		inline OutIterator(const OutIterator& iterator): iter(iterator.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline ograph::Edge *item(void) const { return iter.edge(); }
		inline void next(void) { iter.next(); }
	private:
		Node::Successor iter;
	};
	typedef OutIterator Successor;

	// BiDiGraph concept
	Node *sourceOf(ograph::Edge *edge) const { return edge->source(); }
	int inDegree(Node *vertex) const;
	bool isPredecessorOf(Node *prev, Node *ref) const;

	// InIterator class
	class InIterator: public elm::PreIterator<InIterator, ograph::Edge *> {
	public:
		inline InIterator(const Node *source): iter(source) { }
		inline InIterator(const Graph& graph, const Node *source): iter(source) { }
		inline InIterator(const InIterator& iterator): iter(iterator.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline ograph::Edge *item(void) const { return iter.edge(); }
		inline void next(void) { iter.next(); }
	private:
		Node::Predecessor iter;
	};
	
	// DiGraphWithIndexedVertex concept
	inline int indexOf(const Node *vertex) const { return vertex->index(); }

	// DiGraphWithVertexMAp concept
	template <class T>
	class VertexMap  {
	public:
		inline VertexMap(const Graph& g): _g(g), _tab(g.count()), _set(g.count()) { }
		inline Option<T> get(Vertex key) const { if(hasKey(key)) return some(_tab[key->index()]); else return none; }
		inline const T &get(Vertex key, const T &def) const { if(hasKey(key)) return _tab[key->index()]; else return def; }
		inline bool hasKey(Vertex key) const { return _set.bit(key->index()); }
		void put(Vertex key, const T &value) { _tab[key->index()] = value; _set.set(key->index()); }
		void remove(Vertex key) { _set.clear(key->index()); }

		class KeyIter: public Graph::Iter {
		public:
			inline KeyIter(const VertexMap<T>& m): Graph::Iter(m._g) { }
		};

		class Iter: public PreIterator<Iter, T> {
		public:
			inline Iter(const VertexMap<T>& m): _m(m), _i(m._set) { }
			inline bool ended(void) const { return _i.ended(); }
			inline const T& item(void) const { return _m._tab[_i]; }
			inline void next(void) { _i++; }
		private:
			const VertexMap<T>& _m;
			BitVector::OneIterator _i;
		};

	private:
		const Graph& _g;
		AllocArray<T> _tab;
		BitVector _set;
	};

private:
	elm::FragTable<Node *> nodes;
};


// Node Inlines
inline Node::Node(Graph *graph)
: _graph(0), idx(-1), ins(0), outs(0) {
	if(graph)
		graph->add(this);
}

inline bool Node::isPredOf(const Node *node) {
	for(Successor succ(this); succ(); succ++)
		if(*succ == node)
			return true;
	return false;
}
	
inline bool Node::isSuccOf(const Node *node) {
	for(Predecessor pred(this); pred(); pred++)
		if(*pred == node)
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
	return _edge->sink();
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

