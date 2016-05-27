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

#include <elm/datastruct/Collection.h>
#include <elm/data/List.h>
#include <elm/sys/Path.h>
#include <otawa/display/display.h>
#include <otawa/prop/AbstractIdentifier.h>

namespace otawa { namespace display {

using namespace elm;

class Exception: public otawa::Exception {
public:
	inline Exception(string msg): otawa::Exception(msg) { }
};

class AbstractGraph {
public:

	class Vertex {
	};

	class Edge {
	};

	virtual ~AbstractGraph(void);
	virtual datastruct::IteratorInst<const Vertex *> *vertices(void) const = 0;
	virtual datastruct::IteratorInst<const Edge *> *outs(const Vertex& v) const = 0;
	virtual datastruct::IteratorInst<const Edge *> *ins(const Vertex& v) const = 0;
	virtual const Vertex& sourceOf(const Edge& v) const = 0;
	virtual const Vertex& sinkOf(const Edge& v) const = 0;
	virtual string id(const Vertex& v) const = 0;

};

class Decorator {
public:
	virtual ~Decorator(void);
	virtual void decorate(const AbstractGraph& graph, Text& caption, GraphStyle& style) const;
	virtual void decorate(const AbstractGraph& graph, const AbstractGraph::Vertex& vertex, Text& content, VertexStyle& style) const;
	virtual void decorate(const AbstractGraph& graph, const AbstractGraph::Edge& edge, Text& label, EdgeStyle& style) const;
};

class Displayer {
public:

	Displayer(const AbstractGraph& graph, const Decorator& decorator, output_mode_t out);
	virtual ~Displayer(void);
	virtual void process(void) throw(Exception) = 0;

	inline void setPath(const sys::Path& p) { path = p; }
	inline VertexStyle& defaultVertex(void) { return default_vertex; }
	inline EdgeStyle& defaultEdge(void) { return default_edge; }
	inline void setLayout(layout_t l) { layout = l; }

protected:
	const AbstractGraph& g;
	const Decorator& d;
	output_mode_t output;
	layout_t layout;
	sys::Path path;
	VertexStyle default_vertex;
	EdgeStyle default_edge;
};

class Provider: public AbstractIdentifier {
public:

	Provider(cstring name);
	virtual ~Provider(void);

	virtual bool accepts(output_mode_t out) = 0;
	virtual Displayer *make(const AbstractGraph& g, const Decorator& d, output_mode_t out = OUTPUT_ANY) = 0;

	static Displayer *display(const AbstractGraph& g, const Decorator& d, output_mode_t out = OUTPUT_ANY);
	static Provider *get(cstring name = "");
	static Provider *get(output_mode_t out);

private:
	static List<Provider *> provs;
	static Provider *def;
};

} }	// otawa::display

#endif /* OTAWA_DISPLAY_DISPLAYER_H_ */
