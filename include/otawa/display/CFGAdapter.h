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
#ifndef OTAWA_DISPLAY_CFGADAPTER_H_
#define OTAWA_DISPLAY_CFGADAPTER_H_

#include <otawa/cfg.h>

namespace otawa { namespace display {

// CFGAdapter class
class CFGAdapter {
public:

	// DiGraph concept
	class Vertex {
	public:
		inline Vertex(Block *_b): b(_b) { }
		inline int index(void) { return b->index(); }
		Block *b;
	};

	class Edge {
	public:
		inline Edge(otawa::Edge *_edge): edge(_edge) { }
		inline Vertex source(void) const { return Vertex(edge->source()); }
		inline Vertex sink(void) const { return Vertex(edge->target()); }
		otawa::Edge *edge;
	};
	
	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const CFGAdapter& ad, Vertex source): i(source.b->outs()) { }
		inline bool ended(void) const { return i.ended(); }
		inline Edge item(void) const { return Edge(*i); }
		inline void next(void) { i.next(); }
	private:
		Block::EdgeIter i;
	};
	
	// Collection concept
	class Iter: public PreIterator<Iter, Vertex> {
	public:
		inline Iter(const CFGAdapter& adapter): i(adapter.cfg->blocks()) { }
		inline bool ended(void) const { return i.ended(); }
		inline Vertex item(void) const { return Vertex(*i); }
		inline void next(void) { i.next(); }
	private:
		CFG::BlockIter i;
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap {
	public:
		inline VertexMap(const CFGAdapter& adapter)
			: vals(new T[adapter.count()]) { }
		inline const T& get(const Vertex& vertex) const
			{ return vals[vertex.b->index()]; }
		inline void put(const Vertex& vertex, const T& val)
			{ vals[vertex.b->index()] = val; }
	private:
		T *vals;
	};
	
	inline Vertex sinkOf(Edge e) const { return e.sink(); }
	inline CFGAdapter(CFG *_cfg, WorkSpace *_ws = 0): cfg(_cfg), ws(_ws) { }
	inline int count(void) const{ return cfg->count(); }
	CFG *cfg;
	WorkSpace *ws;
};

} } // otawa::display

#endif // OTAWA_DISPLAY_CFGADAPTER_H_
