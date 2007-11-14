/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 * 
 * otawa/util/GenGraph.h -- GenGraph, GenNode and GenEdge classes interfaces.
 */
#ifndef OTAWA_UTIL_GEN_GRAPH_H
#define OTAWA_UTIL_GEN_GRAPH_H
#include <elm/Iterator.h>
#include <otawa/util/Graph.h>

namespace otawa {

// Predeclaration
template <class N, class E> class GenGraph;
template <class N, class E> class GenNode;
template <class N, class E> class GenEdge;

// GenGraph class
template <class N, class E>
class GenGraph: private graph::Graph {
public:
	typedef N _Node;
	typedef E _Edge;

	// GenNode class
	class Node: private graph::Node {
		friend class GenGraph<N, E>;
	protected:
		inline Node(graph::Graph *graph = 0);
		virtual ~Node(void);
	public:
		inline graph::Graph *graph(void) const;	
		inline int index(void) const;
		inline bool hasSucc(void) const;
		inline bool hasPred(void) const;
		inline int countSucc(void) const;	
		inline int countPred(void) const;
		inline bool isPredOf(const Node *node);
		inline bool isSuccOf(const Node *node);
	};

	// GenEdge class
	class Edge: private graph::Edge {
		friend class GenGraph<N, E>;
	protected:
		virtual ~Edge(void);
	public:
		inline Edge(Node *source, Node *target);
		inline N *source(void) const;
		inline N *target(void) const;
	};

	// Methods
	inline void add(Node *node);
	inline void remove(Node *node);
	inline void destroy(Node *node);
	inline void destroy(Edge *edge);
	inline int count(void) const { return Graph::count(); }

	// Successor class
	class Successor: public elm::PreIterator<Successor, N *> {
		graph::Node::Successor succ;
	public:
		inline Successor(const N *node);
		inline bool ended(void) const;
		inline void next(void);
		inline N *item(void) const;
		inline E *edge(void) const;
	};
	
	// Predecessor class
	class Predecessor: public elm::PreIterator<Predecessor, N *> {
		graph::Node::Predecessor pred;
	public:
		inline Predecessor(const N *node);
		inline bool ended(void) const;
		inline void next(void);
		inline N *item(void) const;
		inline E *edge(void) const;
	};

	// NodeIterator class
	class NodeIterator: public elm::PreIterator<NodeIterator, N *> {
		graph::Graph::NodeIterator iter;
	public:
		inline NodeIterator(const GenGraph<N, E> *graph);
		inline NodeIterator(const GenGraph<N, E>::NodeIterator *graph);
		inline bool ended(void) const;
		inline N *item(void) const;
		inline void next(void); 
	};

	// PreorderIterator class
	class PreorderIterator: public elm::PreIterator<PreorderIterator, N *> {
		graph::Graph::PreorderIterator iter;
	public:
		inline PreorderIterator(const GenGraph<N, E> *graph, N *entry);
		inline bool ended(void) const;
		inline N *item(void) const;
		inline void next(void);
	};

private:
	inline static const graph::Node *_(const Node *node) { return node; }; 
	inline static const graph::Edge *_(const Edge *edge) { return edge; }; 
	inline static graph::Node *_(Node *node) { return node; }; 
	inline static graph::Edge *_(Edge *edge) { return edge; }; 
};


// GenGraph<N, E> Inlines
template <class N, class E>
inline void GenGraph<N, E>::add(Node *node) {
	graph::Graph::add(node);
}

template <class N, class E>
inline void GenGraph<N, E>::remove(Node *node) {
	graph::Graph::remove(node);
}

template <class N, class E>
inline void GenGraph<N, E>::destroy(Node *node) {
	graph::Graph::destroy(node);
}

template <class N, class E>
inline void GenGraph<N, E>::destroy(Edge *edge) {
	graph::Graph::destroy(edge);
}


// GenGraph<N, E>::Node Inlines
template <class N, class E>
inline GenGraph<N, E>::Node::Node(graph::Graph *graph)
: graph::Node(graph) {
}

template <class N, class E>
GenGraph<N, E>::Node::~Node(void) {
}

template <class N, class E>
inline graph::Graph *GenGraph<N, E>::Node::graph(void) const {
	return graph::Node::graph();
}

template <class N, class E>
inline int GenGraph<N, E>::Node::index(void) const {
	return graph::Node::index();
}

template <class N, class E>
inline bool GenGraph<N, E>::Node::hasSucc(void) const {
	return graph::Node::hasSucc();
}

template <class N, class E>
inline bool GenGraph<N, E>::Node::hasPred(void) const {
	return graph::Node::hasPred();
}

template <class N, class E>
inline int GenGraph<N, E>::Node::countSucc(void) const {
	return graph::Node::countSucc();
}

template <class N, class E>
inline int GenGraph<N, E>::Node::countPred(void) const {
	return graph::Node::countPred();
}

template <class N, class E>
inline bool GenGraph<N, E>::Node::isPredOf(const GenGraph<N, E>::Node *node) {
	return graph::Node::countPred();
}

template <class N, class E>
inline bool GenGraph<N, E>::Node::isSuccOf(const GenGraph<N, E>::Node *node) {
	return graph::Node::isSuccOf();
}


// GenGraph<N, E>::Edge Inlines
template <class N, class E>
GenGraph<N, E>::Edge::~Edge(void) {
}

template <class N, class E>
inline GenGraph<N, E>::Edge::Edge(Node *source, Node *target)
: graph::Edge(_(source), _(target)) {
}

template <class N, class E>
inline N *GenGraph<N, E>::Edge::source(void) const {
	return (N *)graph::Edge::source();
}

template <class N, class E>
inline N *GenGraph<N, E>::Edge::target(void) const {
	return (N *)graph::Edge::target();
}


// GenGraph<N, E>::Successor
template <class N, class E>
inline GenGraph<N, E>::Successor::Successor(const N *node)
: succ(_(node)) {
}

template <class N, class E>
inline bool GenGraph<N, E>::Successor::ended(void) const {
	return succ.ended();
}

template <class N, class E>
inline void GenGraph<N, E>::Successor::next(void) {
	succ.next();
}

template <class N, class E>
inline N *GenGraph<N, E>::Successor::item(void) const {
	return (N *)succ.item();
}

template <class N, class E>
inline E *GenGraph<N, E>::Successor::edge(void) const {
	return (E *)succ.edge();
}	


// GenGraph<N, E>::Predecessor
template <class N, class E>
inline GenGraph<N, E>::Predecessor::Predecessor(const N *node)
: pred(_(node)) {
}

template <class N, class E>
inline bool GenGraph<N, E>::Predecessor::ended(void) const {
	return pred.ended();
}

template <class N, class E>
inline void GenGraph<N, E>::Predecessor::next(void) {
	pred.next();
}

template <class N, class E>
inline N *GenGraph<N, E>::Predecessor::item(void) const {
	return (N *)pred.item();
}

template <class N, class E>
inline E *GenGraph<N, E>::Predecessor::edge(void) const {
	return (E *)pred.edge();
}


// GenGraph<N, T>::NodeIterator Inlines
template <class N, class E>
inline GenGraph<N, E>::NodeIterator::NodeIterator(const GenGraph<N, E> *graph)
: iter(graph) {
}

template <class N, class E>
inline GenGraph<N, E>::NodeIterator::NodeIterator(const GenGraph<N, E>::NodeIterator *iterator)
: iter(iterator.iter) {
}

template <class N, class E>
inline bool GenGraph<N, E>::NodeIterator::ended(void) const {
	return iter.ended();
}

template <class N, class E>
inline N *GenGraph<N, E>::NodeIterator::item(void) const {
	return (N *)iter.item();
}

template <class N, class E>
inline void GenGraph<N, E>::NodeIterator::next(void) {
	iter.next();
}


// GenGraph<N, E>::PreorderIterator Inlines
template <class N, class E>
inline GenGraph<N, E>::PreorderIterator::PreorderIterator(
	const GenGraph<N, E> *graph,
	N *entry)
: iter(graph, _(entry)) {
}

template <class N, class E>
inline bool GenGraph<N, E>::PreorderIterator::ended(void) const {
	return iter.ended();
}

template <class N, class E>
inline N *GenGraph<N, E>::PreorderIterator::item(void) const {
	return (N *)iter.item();
}

template <class N, class E>
inline void GenGraph<N, E>::PreorderIterator::next(void) {
	iter.next();
}

} // otawa

#endif	// OTAWA_UTIL_GEN_GRAPH_H
