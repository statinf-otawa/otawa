/*
 *	display::Displayer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_DISPLAY_DISPLAYER_H_
#define OTAWA_DISPLAY_DISPLAYER_H_

#include <elm/data/List.h>
#include <elm/dyndata/Collection.h>
#include <elm/sys/Path.h>
#include <otawa/display/display.h>
#include <otawa/prop/AbstractIdentifier.h>
#include <otawa/graph/DiGraph.h>

namespace otawa { namespace display {

using namespace elm;

class Exception: public otawa::Exception {
public:
	inline Exception(string msg): otawa::Exception(msg) { }
};

class Decorator {
public:
	virtual ~Decorator(void);
	virtual void decorate(graph::DiGraph *graph, Text& caption, GraphStyle& style) const;
	virtual void decorate(graph::DiGraph *graph, graph::Vertex *vertex, Text& content, VertexStyle& style) const;
	virtual void decorate(graph::DiGraph *graph, graph::Edge *edge, Text& label, EdgeStyle& style) const;
};

template <class G>
class GenDecorator: public Decorator {
public:
	typedef typename G::vertex_t vertex_t;
	typedef typename G::edge_t edge_t;

	virtual void decorate(G *graph, Text& caption, GraphStyle& style) const { }
	virtual void decorate(G *graph, vertex_t *vertex, Text& content, VertexStyle& style) const { }
	virtual void decorate(G *graph, edge_t *edge, Text& label, EdgeStyle& style) const { }

	void decorate(graph::DiGraph *graph, Text& caption, GraphStyle& style) const override
		{ decorate(static_cast<G *>(graph), caption, style); }
	void decorate(graph::DiGraph *graph, graph::Vertex *vertex, Text& content, VertexStyle& style) const override
		{ decorate(static_cast<G *>(graph), static_cast<vertex_t *>(vertex), content, style); }
	void decorate(graph::DiGraph *graph, graph::Edge *edge, Text& label, EdgeStyle& style) const override
		{ decorate(static_cast<G *>(graph), static_cast<edge_t *>(edge), label, style); }
};

class Displayer {
public:

	virtual ~Displayer(void);
	virtual void process(void) = 0;

	inline sys::Path path(void) const { return _path; }
	inline layout_t layout(void) const { return _layout; }
	inline output_mode_t output(void) const { return _output; }

	void setPath(const sys::Path& p);
	inline VertexStyle& defaultVertex(void) { return default_vertex; }
	inline EdgeStyle& defaultEdge(void) { return default_edge; }
	inline void setLayout(layout_t l) { _layout = l; }

	static Displayer *make(graph::DiGraph *g, const Decorator& d, output_mode_t out = OUTPUT_ANY);

protected:
	Displayer(graph::DiGraph *graph, const Decorator& decorator, output_mode_t out);

	graph::DiGraph *g;
	const Decorator& d;
	output_mode_t _output;
	layout_t _layout;
	sys::Path _path;
	VertexStyle default_vertex;
	EdgeStyle default_edge;
};

class Provider: public AbstractIdentifier {
public:

	Provider(cstring name);
	virtual ~Provider(void);

	virtual bool accepts(output_mode_t out) = 0;
	virtual Displayer *make(graph::DiGraph *g, const Decorator& d, output_mode_t out = OUTPUT_ANY) = 0;

	static Displayer *display(graph::DiGraph *g, const Decorator& d, output_mode_t out = OUTPUT_ANY);
	static Provider *get(cstring name = "");
	static Provider *get(output_mode_t out);

private:
	static List<Provider *> provs;
	static Provider *def;
};

Displayer *make(graph::DiGraph *g, const Decorator& d, output_mode_t out = OUTPUT_ANY);

} }	// otawa::display

#endif /* OTAWA_DISPLAY_DISPLAYER_H_ */
