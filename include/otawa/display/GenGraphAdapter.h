/*
 *	$Id$
 *	CFGAdapter class interface
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
#ifndef OTAWA_DISPLAY_GENGRAPHADAPTER_H_
#define OTAWA_DISPLAY_GENGRAPHADAPTER_H_

#include <otawa/util/GenGraph.h>

namespace otawa { namespace display {

// GenGraphAdapter class
template <class G>
class GenGraphAdapter {
public:
	
	class Vertex {
	public:
		inline Vertex(typename G::_Node *_node): node(_node) { }
		inline int index(void) { return node->index(); }
		inline typename G::_Node *operator->(void) const { return node; }
		typename G::_Node *node;
	};

	class Edge {
	public:
		inline Edge(typename G::_Edge *_edge): edge(_edge) { }
		inline Vertex source(void) const { return Vertex(edge->source()); }
		inline Vertex sink(void) const { return Vertex(edge->target()); }
		inline typename G::_Edge *operator->(void) const { return edge; }
		typename G::_Edge *edge;
	};
	
	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const GenGraphAdapter& graph, Vertex source)
			: iter(source.node) { }
		inline Successor(const Successor& succ): iter(succ.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline Edge item(void) const { return Edge(iter.edge()); }
		inline void next(void) { iter.next(); }
	private:
		typename G::Successor iter;
	};
	
	class Iterator: public PreIterator<Iterator, Vertex> {
	public:
		inline Iterator(const GenGraphAdapter& adapter): iter(adapter.graph) { }
		inline Iterator(const Iterator& _iter): iter(_iter.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline Vertex item(void) const { return Vertex(iter); }
		inline void next(void) { iter.next(); }
	private:
		typename G::NodeIterator iter;
	};
	
	template <class T>
	class VertexMap {
	public:
		inline VertexMap(const GenGraphAdapter& adapter)
			: vals(new T[adapter.graph->count()]) { }
		inline const T& get(const Vertex& vertex) const
			{ return vals[vertex.node->index()]; }
		inline void put(const Vertex& vertex, const T& val)
			{ vals[vertex.node->index()] = val; }
	private:
		T *vals;
	};
	
	inline GenGraphAdapter(G *_graph): graph(_graph) { }
	inline int count(void) const { return graph->count(); }
	inline G *operator->(void) const { return graph; }
	
	G *graph;
};

} }	// otawa::display

#endif // OTAWA_DISPLAY_GENGRAPHADAPTER_H_
