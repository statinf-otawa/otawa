/*
 *	display::GenGraphDisplayer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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
#ifndef OTAWA_DISPLAY_GENGRAPHDISPLAYER_H
#define OTAWA_DISPLAY_GENGRAPHDISPLAYER_H

#include "Displayer.h"
#include <otawa/graph/GenGraph.h>

namespace otawa { namespace display {

template <class V, class E>
class GenDisplayer {
	typedef graph::GenGraph<V, E> graph_t;

	class MyEdge;
	class MyVertex: public AbstractGraph::Vertex {
	public:
		inline MyVertex(void): _v(0) { }
		inline MyVertex(V *v): _v(v) { }
		const V *_v;
		List<MyEdge *> succs, preds;
	};

	class MyEdge: public AbstractGraph::Edge {
	public:
		inline MyEdge(void): _e(0) { }
		inline MyEdge(E *e): _e(e) { }
		E *_e;
	};

	class Graph: public AbstractGraph {
	public:

		Graph(const graph_t& g): _g(g), _vs(g.count()) {
			for(typename graph_t::Iter v = _g.items(); v; v++)
				_vs[v->index()] = new MyVertex(*v);
			for(typename graph_t::Iter v = _g.items(); v; v++) {
				for(typename graph_t::Successor e(v); e; e++) {
					MyEdge *ae = new MyEdge(e);
					_vs[v->index()]->succs.add(ae);
					_vs[e->source()->index()]->preds.add(ae);
				}
			}
			_init = true;
		}

		~Graph(void) {
			for(int i = 0; i < _vs.count(); i++) {
				for(typename List<MyEdge *>::Iter e = _vs[i]->succs.items(); e; e++)
					delete *e;
				delete _vs[i];
			}
		}

		virtual datastruct::IteratorInst<const Vertex *> *vertices(void) const
			{ return datastruct::abstract_iter<const Vertex *>(_vs.items()); }

		virtual datastruct::IteratorInst<const Edge *> *outs(const Vertex& v) const
			{ return datastruct::abstract_iter<const Edge *>(static_cast<const MyVertex &>(v).succs.items()); }
		virtual datastruct::IteratorInst<const Edge *> *ins(const Vertex& v) const
			{ return datastruct::abstract_iter<const Edge *>(static_cast<const MyVertex &>(v).preds.items()); }

		virtual const Vertex& sourceOf(const Edge& e) const
			{ return *_vs[_g.sourceOf(static_cast<const MyEdge &>(e)._e)->index()]; }
		virtual const Vertex& sinkOf(const Edge& e) const
			{ return *_vs[_g.sinkOf(static_cast<const MyEdge &>(e)._e)->index()]; }
		virtual string id(const Vertex& v) const
			{ return _ << _g.indexOf(static_cast<const MyVertex &>(v)._v); }

		const graph_t& _g;
		mutable AllocArray<MyVertex *> _vs;
		mutable bool _init;
	};

public:

	class Decorator: private display::Decorator {
		friend class GenDisplayer;
	public:
		virtual ~Decorator(void) { }

		virtual void decorate(const graph_t& g, Text& caption, GraphStyle& style) const = 0;
		virtual void decorate(const graph_t& g, const V *v, Text& content, VertexStyle& style) const = 0;
		virtual void decorate(const graph_t& g, const E *e, Text& label, EdgeStyle& style) const = 0;
	private:

		virtual void decorate(const AbstractGraph& g, Text& caption, GraphStyle& style) const
			{  decorate(static_cast<const Graph &>(g)._g, caption, style); }

		virtual void decorate(const AbstractGraph& g, const AbstractGraph::Vertex& v, Text& content, VertexStyle& style) const
			{ decorate(static_cast<const Graph &>(g)._g, static_cast<const MyVertex &>(v)._v, content, style); }

		virtual void decorate(const AbstractGraph& g, const AbstractGraph::Edge& e, Text& label, EdgeStyle& style) const
			{ decorate(static_cast<const Graph &>(g)._g, static_cast<const MyEdge &>(e)._e, label, style); }

	};

public:
	GenDisplayer(const graph_t& g, const Decorator& dec, display::output_mode_t mode = display::OUTPUT_DOT)
		: _g(g), _dec(dec), _mode(mode), _disp(display::Provider::display(_g, dec, mode)) { }
	GenDisplayer(display::Provider *prov, const graph_t& g, const Decorator& dec, display::output_mode_t mode = display::OUTPUT_DOT)
		: _g(g), _dec(dec), _mode(mode), _disp(prov->make(_g, dec, mode)) { }
	virtual ~GenDisplayer(void) { delete _disp; }
	inline display::Displayer *displayer(void) const { return _disp; }
	virtual void process(void) throw(Exception) { _disp->process(); }
	inline void setPath(const sys::Path& p) { _disp->setPath(p); }
	inline VertexStyle& defaultVertex(void) { return _disp->defaultVertex(); }
	inline EdgeStyle& defaultEdge(void) { return _disp->defaultEdge(); }
	inline void setLayout(layout_t l) { _disp->setLayout(l); }

private:
	Graph _g;
	const Decorator& _dec;
	output_mode_t _mode;
	display::Displayer *_disp;
};

} }	// otawa::display

#endif	// OTAWA_DISPLAY_GENGRAPHDISPLAYER_H
