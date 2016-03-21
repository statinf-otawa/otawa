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
#include <elm/avl/Map.h>

namespace otawa { namespace display {

// GenDrawer class
template <class G, class D>
class GenDrawer: public AbstractDrawer {
public:
	inline GenDrawer(const G& graph, bool inlineAll);

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
GenDrawer<G, D>::GenDrawer(const G& graph, bool inlining): _graph(graph) {
	typename G::template VertexMap<Vertex *> map(graph);

	// Process the vertices
	for(typename G::Iterator vertex(graph); vertex; vertex++) {
		Vertex* n = new Vertex(*this, _graph, *vertex);
		map.put(*vertex, n);
	}

	// Process the edges, for each Block, create edges by following all the out-going edges of the Block
	if(inlining) {
		// However, if the inlining mode, the Synth block is not shown, hence we have to bridge the caller block with the entry block of the calleee
		for(typename G::Iterator vertex(graph); vertex; vertex++) {
			for(typename G::Successor edge(_graph, *vertex); edge; edge++) {
				if((*edge).sink().hasCallee()) {
					Vertex *so = map.get(*vertex); // the caller
					Vertex *sn = map.get((*edge).sink().calleeEntry()); // the callee
					new Edge(*this, _graph, so, sn, *edge); // make the entry
					// we also need to take care of the exit block of the callee, to the out-going blocks of the caller
					Vertex *so2 = map.get((*edge).sink().calleeExit()); // the exit block of the callee
					for(typename G::Successor edge2(_graph, (*edge).sink()); edge2; edge2++) { // find the out going blocks from the caller
						Vertex *sn2 = map.get((*edge2).sink()); // for each target block, we create an edge
						new Edge(*this, _graph, so2, sn2, *edge2);
					}
				}
				else {
					Vertex *so = map.get(*vertex);
					Vertex *sn = map.get((*edge).sink());
					new Edge(*this, _graph, so, sn, *edge);
				}
			}
		}
	} // end inlineAll
	else {
		// Process the edges
		for(typename G::Iterator vertex(graph); vertex; vertex++)
			for(typename G::Successor edge(_graph, *vertex); edge; edge++) {
				Vertex *so = map.get(*vertex);
				Vertex *sn = map.get((*edge).sink());
				new Edge(*this, _graph, so, sn, *edge);
			}
	} // end of edge processing
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
