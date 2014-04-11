/*
 *	ai module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_DFA_AI_H_
#define OTAWA_DFA_AI_H_

#include <elm/genstruct/Vector.h>
#include <elm/util/Pair.h>
#include <elm/util/BitVector.h>
#include <elm/PreIterator.h>
#include <otawa/cfg.h>

namespace otawa { namespace ai {

using namespace elm;
using namespace elm::genstruct;

/**
 * Driver of abstract interpretation with a simple to-do list.
 * @param D		Current domain (must implement otawa::ai::Domain concept).
 * @param G		Graph (must implement otawa::ai::Graph concept).
 * @param S		Storage.
 */
template <class D, class G, class S>
class WorkListDriver: public PreIterator<WorkListDriver<D, G, S>, typename G::vertex_t > {
public:
	typedef typename G::vertex_t vertex_t;

	/**
	 * Initialize the driver.
	 * @param dom	Domain.
	 * @param graph	Graph to analyze.
	 * @param store	Storage to get domain values.
	 */
	WorkListDriver(D& dom, const G& graph, S& store)
	: _dom(dom), _graph(graph), _store(store), wl_set(graph.count()), end(false) {
		store.set(_graph.entry(), dom.init());
		for(typename G::Successor succ(graph, _graph.entry()); succ; succ++)
			push(graph.sinkOf(*succ));
		next();
	}

	/**
	 * Test if the traversal is ended.
	 * @return	True if ended, false else.
	 */
	inline bool ended(void) {
		return end;
	}

	/**
	 * Go to the next vertex to process.
	 */
	inline void next(void) {
		while(!wl_vertices.isEmpty()) {
			cur = pop();
			if(cur != _graph.exit())
				return;
		}
		end = true;
	}

	/**
	 * Get the current vertex.
	 * @return	Current vertex.
	 */
	inline typename G::vertex_t item(void) {
		return cur;
	}

	/**
	 * Called when the output state of the current vertex is changed
	 * (and successors must be updated).
	 */
	inline void change(void) {
		for(typename G::Successor succ(_graph, cur); succ; succ++)
			push(*succ);
	}

	/**
	 * Set the new output state of the current vertex.
	 */
	inline void change(typename D::t s) {
		_store.set(cur, s);
		change();
	}

	/**
	 * Called when the output state of the current vertex for the given edge is changed
	 * (and successors must be updated).
	 */
	inline void change(typename G::edge_t edge) {
		push(_graph.sinkOf(edge));
	}

	/**
	 * Change the output value of the current vertex for the given edge.
	 */
	inline void change(typename G::edge_t edge, typename D::t s) {
		_store.set(edge, s);
		change(edge);
	}

	/**
	 * Consider that all has been changed causing a re-computation of all.
	 */
	inline void changeAll(void) {

		// collect vertices
		wl_vertices.clear();
		genstruct::Vector<vertex_t> todo;
		todo.push(_graph.entry());
		while(todo) {
			vertex_t v = todo.pop();
			for(typename G::Successor s(_graph, v); s; s++)
				if(!contains(_graph.sinkOf(s))) {
					push(_graph.sinkOf(s));
					todo.push(_graph.sinkOf(s));
				}
		}

		// reverse the order
		for(int i = 0, j = wl_vertices.length() - 1; i < j; i++, j--) {
			vertex_t tmp = wl_vertices[i];
			wl_vertices[i] = wl_vertices[j];
			wl_vertices[j] = tmp;
		}

		// and start...
		next();
	}

	/**
	 * If there is a state change for the given edge.
	 */
	inline void check(typename G::edge_t edge, typename D::t s) {
		typename D::t ps = _store.get(edge);
		if(!_dom.equals(s, ps))
			change(edge, s);
	}

	/**
	 * Compute the input state.
	 */
	inline typename D::t input(void) {
		return input(cur);
	}

	/**
	 * Compute the input state for the given vertex.
	 * @param 	vertex	Vertex whose input is required.
	 * @return			Input state of vertex.
	 */
	inline typename D::t input(vertex_t vertex) {
		typename D::t s = _dom.bot();
		for(typename G::Predecessor pred(_graph, vertex); pred; pred++) {
			s = _dom.join(s, _store.get(*pred));
		}
		return s;
	}

private:

	inline void push(typename G::vertex_t v) {
		if(!wl_set.bit(_graph.index(v))) {
			wl_vertices.push(v);
			wl_set.set(_graph.index(v));
		}
	}

	inline typename G::vertex_t pop(void) {
		typename G::vertex_t v = wl_vertices.pop();
		wl_set.clear(_graph.index(v));
		return v;
	}

	inline bool contains(vertex_t v) {
		return wl_set.bit(_graph.index(v));
	}

	D& _dom;
	const G& _graph;
	S& _store;
	Vector<typename G::vertex_t> wl_vertices;
	BitVector wl_set;
	typename G::vertex_t cur;
	bool end;
};


/**
 * BiDiGraph implementation for CFG.
 */
class CFGGraph {
public:
	inline CFGGraph(CFG *cfg): _cfg(cfg) { }

	// BiDiGraph concept
	typedef BasicBlock *vertex_t;
	typedef Edge *edge_t;

	inline vertex_t entry(void) const { return _cfg->entry(); }
	inline vertex_t exit(void) const { return _cfg->exit(); }
	inline vertex_t sinkOf(edge_t e) const { return e->target(); }
	inline vertex_t sourceOf(edge_t e) const { return e->source(); }

	class Predecessor: public BasicBlock::InIterator {
	public:
		inline Predecessor(const CFGGraph& graph, vertex_t v): BasicBlock::InIterator(v) { }
		inline Predecessor(const Predecessor& p): BasicBlock::InIterator(p) { }
	};

	class Successor: public BasicBlock::OutIterator {
	public:
		inline Successor(const CFGGraph& graph, vertex_t v): BasicBlock::OutIterator(v) { }
		inline Successor(const Successor& p): BasicBlock::OutIterator(p) { }
	};

	class Iterator: public CFG::BBIterator {
	public:
		inline Iterator(const CFGGraph& g): CFG::BBIterator(g._cfg) { }
		inline Iterator(const Iterator& i): CFG::BBIterator(i) { }
	};

	// Indexed concept
	inline int index(vertex_t v) const { return v->number(); }
	inline int count(void) const { return _cfg->countBB(); }

private:
	CFG *_cfg;
};


/**
 * Stockage of output values as an array.
 * @param D		Domain type.
 * @param G		Graph type.
 */
template <class D, class G>
class ArrayStore {
public:
	typedef typename D::t t;
	typedef typename G::vertex_t vertex_t;
	typedef typename G::edge_t edge_t;

	ArrayStore(D& dom, G& graph): _dom(dom), _graph(graph), map(new typename D::t[_graph.count()]) {
		for(int i = 0; i < graph.count(); i++)
			map[i] = dom.bot();
	}

	inline void set(vertex_t v, t s) { map[_graph.index(v)] = s; }

	void set(edge_t e, t s) {
		int i = _graph.index(_graph.sourceOf(e));
		map[i] = _dom.join(map[i], s);
	}

	inline t get(vertex_t v) const { return map[_graph.index(v)]; }

	inline t get(edge_t e) const { return map[_graph.index(_graph.sourceOf(e))]; }

private:
	D& _dom;
	G& _graph;
	typename D::t *map;
};


/**
 * State storage on the edges.
 */
template <class D, class G>
class EdgeStore {
public:
	typedef typename D::t t;
	typedef typename G::vertex_t vertex_t;
	typedef typename G::edge_t edge_t;

	EdgeStore(D& dom, G& graph): _dom(dom), _graph(graph) {
		for(typename G::Successor e(_graph, _graph.entry()); e; e++)
			map.put(*e, _dom.init());
	}

	void set(vertex_t v, t s) {
		for(typename G::Successor e(_graph, v); e; e++)
			map.put(*e, s);
	}

	inline void set(edge_t e, t s) { map.put(e, s); }

	t get(vertex_t v) const {
		t s = _dom.bot();
		for(typename G::Successor e(_graph, v); e; e++)
			s = _dom.join(s, map.get(e, _dom.bot()));
		return s;
	}

	inline t get(edge_t e) { return map.get(e, _dom.bot()); }

private:
	D& _dom;
	G& _graph;
	HashTable<edge_t, t> map;
};

} }		// ai

#endif /* OTAWA_DFA_AI_H_ */
