/*
 *	$Id$
 *	GenDrawer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_DISPLAY_GENDRAWER_H
#define OTAWA_DISPLAY_GENDRAWER_H

#include <otawa/display/AbstractDrawer.h>

namespace otawa { namespace display {

// GenDrawer class
template <class G, class D>
class GenDrawer: public AbstractDrawer {
public:
	inline GenDrawer(const G& graph);

private:
	
	class Vertex: public AbstractDrawer::Vertex {
	public:
		inline Vertex(AbstractDrawer& drawer, const G& graph, typename G::Vertex vertex)
			: AbstractDrawer::Vertex(drawer), _(vertex), _graph(graph) { }
		virtual void configure(Output& content, ShapeStyle& shape)
			{ D::decorate(_graph, _, content, shape); }
	private:
		typename G::Vertex _;
		const G& _graph;
	};
	
	class Edge: public AbstractDrawer::Edge {
	public:
		inline Edge(AbstractDrawer& drawer, const G& graph, Vertex *source, Vertex *sink,
			typename G::Edge edge)
			: AbstractDrawer::Edge(drawer, source, sink), _(edge), _graph(graph) { }
		virtual void configure(Output& label, TextStyle& text, LineStyle& line)
			{ D::decorate(_graph, _, label, text, line); }		
	private:
		typename G::Edge _;
		const G& _graph;
	};
	
	const G& _graph;
	virtual void configure(Output& caption, TextStyle& text, FillStyle& fill)
		{ D::decorate(_graph, caption, text, fill); }
};

// GenDrawer::GenDrawer constructor
template <class G, class D>
GenDrawer<G, D>::GenDrawer(const G& graph): _graph(graph) {
	typename G::template VertexMap<Vertex *> map(graph);
	
	// Process the vertices
	for(typename G::Iterator vertex(graph); vertex; vertex++)
		map.put(vertex, new Vertex(*this, _graph, vertex));
	
	// Process the edges
	for(typename G::Iterator vertex(graph); vertex; vertex++)
		for(typename G::Successor edge(_graph, vertex); edge; edge++)
			new Edge(*this, _graph, map.get(vertex), map.get((*edge).sink()), *edge); 
}


// DefaultDecorator class
template <class G>
class DefaultDecorator {
public:
	static inline void decorate(
		const G& graph,
		Output& caption,
		TextStyle& text,
		FillStyle& fill) { }
	
	static inline void decorate(
		const G& graph,
		const typename G::Vertex vertex,
		Output& content,
		ShapeStyle& style) { content << vertex; }
	
	static inline void decorate(
		const G& graph,
		const typename G::Edge edge,
		Output& label,
		TextStyle& text,
		LineStyle& line) { label << edge; }
};

} } // otawa::display

#endif /* OTAWA_DISPLAY_GENDRAWER_H */
