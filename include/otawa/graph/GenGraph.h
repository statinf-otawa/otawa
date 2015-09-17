/*
 *	$Id$
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
#include <otawa/graph/Graph.h>

// GCC work-around
#if defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 0
#	define OTAWA_GCAST(t, e) ((t)(e))	
#else
#	define OTAWA_GCAST(t, e) static_cast<t>(e)
#endif

namespace otawa {

// GenGraph class
template <class N, class E>
class GenGraph: private graph::Graph {
public:
	typedef N *Vertex;
	typedef E *Edge;

	// GenNode class
	class GenNode: private graph::Node {
		friend class GenGraph<N, E>;
	protected:
		inline GenNode(/*GenGraph<N, E>*/ graph::Graph *graph = 0): graph::Node(graph) { }
		virtual ~GenNode(void) { }
	public:
		//inline graph::Graph *graph(void) const { return graph::Node::graph(); }
		inline int index(void) const { return graph::Node::index(); }
		inline bool hasSucc(void) const { return graph::Node::hasSucc(); }
		inline bool hasPred(void) const { return graph::Node::hasPred(); }
		inline int countSucc(void) const { return graph::Node::countSucc(); }	
		inline int countPred(void) const { return graph::Node::countPred(); }
		inline bool isPredOf(const GenNode *node) { return graph::Node::countPred(); }
		inline bool isSuccOf(const GenNode *node) { return graph::Node::isSuccOf(); }
	};

	// GenEdge class
	class GenEdge: private graph::Edge {
		friend class GenGraph<N, E>;
	protected:
		virtual ~GenEdge(void) { }
	public:
		inline GenEdge(GenNode *source, GenNode *target): graph::Edge(_(source), _(target)) { }
		inline N *source(void) const { return OTAWA_GCAST(N *, graph::Edge::source()); }
		inline N *target(void) const { return OTAWA_GCAST(N *, graph::Edge::target()); }
	};

	// Collection concept
	inline int count(void) const { return Graph::count(); }
	inline bool contains(N *item) const { return Graph::contains(item); }
	inline bool isEmpty(void) const { return Graph::isEmpty(); }
 	inline operator bool(void) const { return !isEmpty(); }
	
	// Iterator class
	class Iterator: public elm::PreIterator<Iterator, N *> {
		graph::Graph::Iterator iter;
	public:
		inline Iterator(const GenGraph<N, E> *graph): iter(graph) { }
		inline Iterator(const GenGraph<N, E>::Iterator& iterator): iter(iterator.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline N *item(void) const  { return OTAWA_GCAST(N *, iter.item()); }
		inline void next(void) { iter.next(); }
	};

	// MutableCollection concept
	inline void clear(void) { graph::Graph::clear(); }
	inline void add(GenNode *node) { graph::Graph::add(node); }
	template <template <class _> class C> void addAll(const C<N *> &items)
		{ graph::Graph::addAll(items); }
	inline void remove(GenNode *node) { graph::Graph::remove(node); }
	template <template <class _> class C> void removeAll(const C<N *> &items)
		{ graph::Graph::removeAll(items); }
	inline void remove(GenEdge *edge) { graph::Graph::remove(edge); }

	// DiGraph concept
	inline N *sinkOf(E *edge) const { return Graph::sinkOf(edge); }
	inline int outDegree(N *vertex) const { return Graph::outDegree(vertex); }
	inline bool isSuccessorOf(N *succ, N *ref) const { return Graph::isSuccessorOf(succ, ref); }

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
	
	// DiGraph concept
	inline N *sourceOf(E *edge) const { return Graph::sourceOf(edge); }
	inline int inDegree(N *vertex) const { return Graph::inDegree(vertex); }
	inline bool isPredecessorOf(N *succ, N *ref) const { return Graph::isPredecessorOf(succ, ref); }

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

	// DiGraphWithIndexedVertex concept
	inline int indexOf(N *vertex) const { return Graph::indexOf(vertex); }

	// private
	inline static const graph::Node *_(const GenNode *node) { return node; }; 
	inline static const graph::Edge *_(const GenEdge *edge) { return edge; }; 
	inline const graph::Graph *_(void) const { return this; }; 
	inline static graph::Node *_(GenNode *node) { return node; }; 
	inline static graph::Edge *_(Edge *edge) { return edge; }; 
	inline graph::Graph *_(void) { return this; }
};

} // otawa

#endif	// OTAWA_UTIL_GRAPH_GRAPH_H
