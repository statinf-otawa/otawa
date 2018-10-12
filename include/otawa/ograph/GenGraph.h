/*
 *	GenGraph class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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
#ifndef OTAWA_GRAPH_GEN_GRAPH_H
#define OTAWA_GRAPH_GEN_GRAPH_H
#include <elm/PreIterator.h>

#include "../ograph/Graph.h"

// GCC work-around
#if defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 0
#	define OTAWA_GCAST(t, e) ((t)(e))	
#else
#	define OTAWA_GCAST(t, e) static_cast<t>(e)
#endif

namespace otawa { namespace ograph {

// GenGraph class
template <class N, class E>
class GenGraph {
public:
	typedef GenGraph<N, E> self_t;
	typedef N *Vertex;
	typedef E *Edge;

	// GenNode class
	class GenNode: private ograph::Node {
		friend class GenGraph<N, E>;
	protected:
		inline GenNode(GenGraph<N, E> *g = 0): ograph::Node(&g->_g) { }
		virtual ~GenNode(void) { }
	public:
		//inline graph::Graph *graph(void) const { return graph::Node::graph(); }
		inline int index(void) const { return ograph::Node::index(); }
		inline bool hasSucc(void) const { return ograph::Node::hasSucc(); }
		inline bool hasPred(void) const { return ograph::Node::hasPred(); }
		inline int countSucc(void) const { return ograph::Node::countSucc(); }	
		inline int countPred(void) const { return ograph::Node::countPred(); }
		inline bool isPredOf(const GenNode *node) { return ograph::Node::isPredOf(node); }
		inline bool isSuccOf(const GenNode *node) { return ograph::Node::isSuccOf(node); }
	};

	// GenEdge class
	class GenEdge: private ograph::Edge {
		friend class GenGraph<N, E>;
	protected:
		virtual ~GenEdge(void) { }
	public:
		inline GenEdge(GenNode *source, GenNode *target): ograph::Edge(_(source), _(target)) { }
		inline N *source(void) const { return OTAWA_GCAST(N *, ograph::Edge::source()); }
		inline N *target(void) const { return OTAWA_GCAST(N *, ograph::Edge::target()); }
	};

	// Collection concept
	inline int count(void) const { return _g.count(); }
	inline bool contains(N *item) const { return _g.contains(item); }
	inline bool isEmpty(void) const { return _g.isEmpty(); }
 	inline operator bool(void) const { return !isEmpty(); }
 	inline N *at(int i) const { return OTAWA_GCAST(N *, at(i)); }
	
	// Iterator class
	class Iter: public elm::PreIterator<Iter, N *> {
		ograph::Graph::Iter iter;
	public:
		inline Iter(const GenGraph<N, E> *graph): iter(&graph->_g) { }
		inline Iter(const GenGraph<N, E>& graph): iter(&graph._g) { }
		inline bool ended(void) const { return iter.ended(); }
		inline N *item(void) const  { return OTAWA_GCAST(N *, iter.item()); }
		inline void next(void) { iter.next(); }
	};
	inline Iter nodes(void) const { return Iter(this); }
	inline Iter items(void) const { return nodes(); }
	inline Iter operator*(void) const { return nodes(); }
	inline operator Iter(void) const { return nodes(); }

	// MutableCollection concept
	inline void clear(void) { _g.clear(); }
	inline void add(GenNode *node) { _g.add(node); }
	template <template <class _> class C> void addAll(const C<N *> &items)
		{ _g.addAll(items); }
	inline void remove(GenNode *node) { _g.remove(node); }
	template <template <class _> class C> void removeAll(const C<N *> &items)
		{ _g.removeAll(items); }
	inline void remove(GenEdge *edge) { _g.remove(edge); }

	// DiGraph concept
	inline N *sinkOf(E *edge) const { return OTAWA_GCAST(N *, _g.sinkOf(edge)); }
	inline int outDegree(N *vertex) const { return _g.outDegree(vertex); }
	inline bool isSuccessorOf(N *succ, N *ref) const { return _g.isSuccessorOf(succ, ref); }

	// OutIterator class
	class OutIterator: public elm::PreIterator<OutIterator, E *> {
	public:
		inline OutIterator(const N *node): iter(_(node)) { }
		inline OutIterator(const GenGraph<N, E>& graph, const N *node): iter(_(node)) { }
		inline bool ended(void) const { return iter.ended(); }
		inline void next(void) { iter.next(); }
		inline E *item(void) const { return OTAWA_GCAST(E *, iter.item()); }
	private:
		Graph::OutIterator iter;
	};
	typedef OutIterator Successor;
	
	// DiGraph concept
	inline N *sourceOf(E *edge) const { return OTAWA_GCAST(N *, _g.sourceOf(edge)); }
	inline int inDegree(N *vertex) const { return _g.inDegree(vertex); }
	inline bool isPredecessorOf(N *succ, N *ref) const { return _g.isPredecessorOf(succ, ref); }

	// InIterator class
	class InIterator: public elm::PreIterator<InIterator, E *> {
	public:
		inline InIterator(const N *node): iter(_(node)) { }
		inline InIterator(const GenGraph<N, E>& graph, const N *node): iter(_(node)) { }
		inline bool ended(void) const { return iter.ended(); }
		inline void next(void) { iter.next(); }
		inline E *item(void) const { return OTAWA_GCAST(E *, iter.item()); }
	private:
		Graph::InIterator iter;
	};
	typedef InIterator Predecessor;

	// DiGraphWithIndexedVertex concept
	inline int indexOf(const N *vertex) const { return _g.indexOf(vertex); }

	template <class T>
	class VertexMap: public Graph::VertexMap<T> {
	public:
		inline VertexMap(const self_t& graph): Graph::VertexMap<T>(graph._g) { }
		inline Option<T> get(Vertex key) const { return Graph::VertexMap<T>::get(key); }
		inline const T &get(Vertex key, const T &def) const { if(hasKey(key)) return (*this)[key->index()]; else return def; }
		inline bool hasKey(Vertex key) const { return Graph::VertexMap<T>::hasKey(key); }
		void put(Vertex key, const T &value) { Graph::VertexMap<T>::put(key, value); }
		void remove(Vertex key) { Graph::VertexMap<T>::remove(key); }
		typedef GenGraph<N, E>::Iter KeyIterator;
	};

	// private
	inline static const ograph::Node *_(const GenNode *node) { return node; }; 
	inline static const ograph::Edge *_(const GenEdge *edge) { return edge; }; 
	inline const ograph::Graph *_(void) const { return this; }; 
	inline static ograph::Node *_(GenNode *node) { return node; }; 
	inline static ograph::Edge *_(Edge *edge) { return edge; }; 
	inline ograph::Graph *_(void) { return this; }

private:
	ograph::Graph _g;
};

} }	// otawa::graph

#endif	// OTAWA_UTIL_GRAPH_GRAPH_H
