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
		inline Vertex(BasicBlock *_bb): bb(_bb) { }
		inline int index(void) { return bb->number(); }
		BasicBlock *bb;
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
		inline Successor(const CFGAdapter& ad, Vertex source): iter(source.bb)
			{ step(); }
		inline Successor(const Successor& succ): iter(succ.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline Edge item(void) const { return Edge(*iter); }
		inline void next(void) { iter.next(); step(); }
	private:
		void step(void) {
			while(!iter.ended() && iter->kind() == otawa::Edge::CALL)
				iter.next();
		}
		BasicBlock::OutIterator iter;
	};
	
	// Collection concept
	class Iterator: public PreIterator<Iterator, Vertex> {
	public:
		inline Iterator(const CFGAdapter& adapter): iter(adapter.cfg) { }
		inline Iterator(const Iterator& _iter): iter(_iter.iter) { }
		inline bool ended(void) const { return iter.ended(); }
		inline Vertex item(void) const { return Vertex(iter); }
		inline void next(void) { iter.next(); }
	private:
		CFG::BBIterator iter;
	};
	
	// DiGraphWithVertexMap concept
	template <class T>
	class VertexMap {
	public:
		inline VertexMap(const CFGAdapter& adapter)
			: vals(new T[adapter.count()]) { }
		inline const T& get(const Vertex& vertex) const
			{ return vals[vertex.bb->number()]; }
		inline void put(const Vertex& vertex, const T& val)
			{ vals[vertex.bb->number()] = val; }
	private:
		T *vals;
	};
	
	inline CFGAdapter(CFG *_cfg, WorkSpace *_ws = 0): cfg(_cfg), ws(_ws) { }
	inline int count(void) const{ return cfg->countBB(); }
	CFG *cfg;
	WorkSpace *ws;
};

} } // otawa::display

#endif // OTAWA_DISPLAY_CFGADAPTER_H_
