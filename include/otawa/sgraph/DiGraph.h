/*
 *	DiGraph class interface
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

#ifndef OTAWA_SIGRAPH_DIGRAPH_H_
#define OTAWA_SIGRAPH_DIGRAPH_H_

#include <elm/data/List.h>
#include <elm/genstruct/FragTable.h>

namespace otawa {

using namespace elm;

namespace sgraph {


// basic classes
class DiGraph;
class Edge;
class Vertex;

class Edge {
	friend class DiGraphBuilder;
public:
	inline Vertex *source(void) const { return src; }
	inline Vertex *sink(void) const { return snk; }
protected:
	~Edge(void);
private:
	Vertex *src, *snk;
};

class Vertex {
	friend class Edge;
	friend class DiGraphBuilder;
public:
	typedef elm::List<Edge *> edges_t;

	inline Vertex(void): idx(0) { }
	inline int index(void) const { return idx; }

	typedef edges_t::Iter EdgeIter;
	inline const edges_t& inEdges(void) const { return _ins; }
	inline const edges_t& outEdges(void) const { return _outs; }
	inline EdgeIter ins(void) const { return _ins.begin(); }
	inline EdgeIter outs(void) const { return _outs.begin(); }
	inline int countIns(void) const { return _ins.count(); }
	inline int countOuts(void) const { return _outs.count(); }

protected:
	~Vertex(void);

private:
	int idx;
	edges_t _ins, _outs;
};

class DiGraph {
	friend class DiGraphBuilder;
	typedef genstruct::FragTable<Vertex *> v_t;
public:
	inline Vertex *entry(void) const { return e; }
	typedef v_t::Iterator VertexIter;
	inline VertexIter vertices(void) const { return VertexIter(v); }
	inline int count(void) const { return v.count(); }
	inline Vertex *at(int index) const { return v[index]; }
private:
	Vertex *e;
	v_t v;
};

class DiGraphBuilder {
public:
	DiGraphBuilder(Vertex *e);
	DiGraphBuilder(DiGraph *g, Vertex *e);
	inline Vertex *entry(void) const { return _g->entry(); }
	void add(Vertex *vertex);
	void add(Vertex *source, Vertex *sink, Edge *edge);
	DiGraph *build(void);
private:
	DiGraph *_g;
	int c;
};


// generic classes
template <class V, class E>
class GenEdge: public Edge {
public:
	inline V *source(void) const { return static_cast<V *>(Edge::source()); }
	inline V *sink(void) const { return static_cast<V *>(Edge::sink()); }
};

template <class V, class E>
class GenVertex: public Vertex {
public:

	class EdgeIter: public PreIterator<EdgeIter, E *> {
		friend class GenVertex<V, E>;
	public:
		EdgeIter(void) { }
		EdgeIter(const EdgeIter& it): i(it.i) { }
		inline bool ended(void) const { return i.ended(); }
		inline E *item(void) const { return static_cast<E *>(*i); }
		inline void next(void) { i.next(); }
	private:
		EdgeIter(const Vertex::EdgeIter& it): i(it) { }
		Vertex::EdgeIter i;
	};

	class EdgeCollection {
	public:
		inline EdgeCollection(const edges_t& edges): _edges(edges) { }
	private:
		inline EdgeIter begin(void) const { return EdgeIter(_edges.begin()); }
		inline EdgeIter end(void) const { return EdgeIter(_edges.end()); }
		const edges_t& _edges;
	};

	inline EdgeCollection inEdges(void) const { return EdgeCollection(Vertex::inEdges()); }
	inline EdgeCollection outEdges(void) const { return EdgeCollection(Vertex::outEdges()); }

	class SuccIter: public PreIterator<SuccIter, V *> {
	public:
		inline SuccIter(void) { }
		inline SuccIter(const Vertex::EdgeIter& i): _i(i) { }
		inline bool ended(void) const { return _i.ended(); }
		inline V *item(void) const { return _i->sink(); }
		inline void next(void) { _i.next(); }
		inline bool operator==(const SuccIter& i) const { return _i == i._i; }
		inline bool operator!=(const SuccIter& i) const { return !operator==(i); }
	private:
		Vertex::EdgeIter _i;
	};

	class SuccCollection {
	public:
		inline SuccCollection(const V *v): _v(v) { }
		inline SuccIter begin(void) const { return SuccIter(_v->outs()); }
		inline SuccIter end(void) const { return SuccIter(); }
	private:
		const V *_v;
	};

	inline SuccCollection succs(void) const { return SuccCollection(this); }

	class PredIter: public PreIterator<SuccIter, V *> {
	public:
		inline PredIter(void) { }
		inline PredIter(const Vertex::EdgeIter& i): _i(i) { }
		inline bool ended(void) const { return _i.ended(); }
		inline V *item(void) const { return _i->source(); }
		inline void next(void) { _i.next(); }
		inline bool operator==(const PredIter& i) const { return _i == i._i; }
		inline bool operator!=(const PredIter& i) const { return !operator==(i); }
	private:
		Vertex::EdgeIter _i;
	};

	class PredCollection {
	public:
		inline PredCollection(const V *v): _v(v) { }
		inline PredIter begin(void) const { return PredIter(_v->ins()); }
		inline PredIter end(void) const { return PredIter(); }
	private:
		const V *_v;
	};

	inline PredCollection preds(void) const { return PredCollection(this); }

	inline EdgeIter ins(void) const { return EdgeIter(Vertex::ins()); }
	inline EdgeIter outs(void) const { return EdgeIter(Vertex::outs()); }
};

template <class V, class E>
class GenDiGraph: public DiGraph {
public:

	class VertexIter: public PreIterator<VertexIter, V *> {
		friend class GenDiGraph<V, E>;
	public:
		inline VertexIter(void) { }
		inline VertexIter(const VertexIter& it): i(it.i) { }
		inline bool ended(void) const { return i.ended(); }
		inline V *item(void) const { return static_cast<V *>(*i); }
		inline void next(void) { i.next(); }
	private:
		inline VertexIter(const DiGraph::VertexIter& it): i(it) { }
		DiGraph::VertexIter i;
	};

	inline V *entry(void) const { return static_cast<V *>(DiGraph::entry()); }
	inline VertexIter vertices(void) const { return VertexIter(DiGraph::vertices()); }
	inline V *at(int index) const { return static_cast<V *>(DiGraph::at(index)); }

};

template <class V, class E>
class GenDiGraphBuilder: private DiGraphBuilder {
public:
	inline GenDiGraphBuilder(V *entry): DiGraphBuilder(entry) { }
	inline GenDiGraphBuilder(GenDiGraph<V, E> *g, V *entry): DiGraphBuilder(g, entry) { }
	inline V *entry(void) const { return static_cast<V *>(DiGraphBuilder::entry()); }
	inline void add(V *v) { DiGraphBuilder::add(v); }
	inline void add(V *v, V *w, E *e) { DiGraphBuilder::add(v, w, e); }
	inline GenDiGraph<V, E> *build(void) { return static_cast<GenDiGraph<V, E> *>(DiGraphBuilder::build()); }
};

} }		// otawa::sgraph

#endif /* OTAWA_SIGRAPH_DIGRAPH_H_ */
