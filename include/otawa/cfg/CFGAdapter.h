/*
 *	$Id$
 *	ForwardCFGAdapter and BackwardCFGAdapter classes interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
#ifndef OTAWA_CFG_CFGADAPTER_H_
#define OTAWA_CFG_CFGADAPTER_H_

#include <otawa/cfg.h>
#include <otawa/cfg/features.h>

namespace otawa {

class ForwardCFGAdapter {
public:

	typedef Block *Vertex;
	typedef otawa::Edge *Edge;

	inline ForwardCFGAdapter(CFG *_cfg): cfg(_cfg) { }
	inline int count(void) const { return cfg->count(); }
	inline Vertex entry(void) const { return cfg->entry(); }
	inline int index(Vertex v) const { return v->index(); }
	inline Vertex sourceOf(Edge edge) const { return edge->source(); }
	inline Vertex sinkOf(Edge edge) const { return edge->target(); }
	inline int outDegree(Vertex vertex) const { int cnt = 0; for(Successor s(*this, vertex); s(); s++) cnt++; return cnt; }
	inline bool isSuccessorOf(Vertex succ, const Vertex& ref) { for(Successor s(*this, ref); s(); s++) if(sinkOf(*s) == succ) return true; return false; }
	inline int inDegree(Vertex vertex) const { int cnt = 0; for(Predecessor s(*this, vertex); s(); s++) cnt++; return cnt; }
	inline bool isPredecessorOf(Vertex pred, const Vertex& ref) { for(Predecessor s(*this, ref); s(); s++) if(sourceOf(*s) == pred) return true; return false; }
	inline bool isLoopHeader(Vertex v) const { return LOOP_HEADER(v); }

	class Predecessor: public PreIterator<Predecessor, Edge> {
	public:
		inline Predecessor(const ForwardCFGAdapter& g, const Vertex& v): iter(v->ins()) { }
		inline bool ended (void) const { return iter.ended(); }
		const Edge item (void) const { return iter.item(); }
		void next(void) { iter++; }
	private:
		Block::EdgeIter iter;
	};

	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const ForwardCFGAdapter& g, const Vertex& v): iter(v->outs()) { }
		inline bool ended (void) const { return iter.ended(); }
		const Edge item (void) const { return iter.item(); }
		void next(void) { iter++; }
	private:
		Block::EdgeIter iter;
	};

	class Iterator: public CFG::BlockIter {
	public:
		inline Iterator(const ForwardCFGAdapter& cfga): CFG::BlockIter(cfga.cfg->blocks()) { }
	};

private:
	CFG *cfg;
};


class BackwardCFGAdapter {
public:

	typedef Block *Vertex;
	typedef otawa::Edge *Edge;

	inline BackwardCFGAdapter(CFG *_cfg): cfg(_cfg) { }
	inline int count(void) const { return cfg->count(); }
	inline Vertex entry(void) const { return cfg->exit(); }
	inline int index(Vertex v) const { return v->index(); }
	inline Vertex sourceOf(Edge edge) const { return edge->target(); }
	inline Vertex sinkOf(Edge edge) const { return edge->source(); }
	inline int outDegree(Vertex vertex) const { int cnt = 0; for(Successor s(*this, vertex); s(); s++) cnt++; return cnt; }
	inline bool isSuccessorOf(Vertex succ, const Vertex& ref) { for(Successor s(*this, ref); s(); s++) if(sinkOf(*s) == succ) return true; return false; }
	inline int inDegree(Vertex vertex) const { int cnt = 0; for(Predecessor s(*this, vertex); s(); s++) cnt++; return cnt; }
	inline bool isPredecessorOf(const Vertex& pred, const Vertex& ref) { for(Predecessor s(*this, ref); s(); s++) if(sourceOf(*s) == pred) return true; return false; }
	inline bool isLoopHeader(Vertex v) const { return LOOP_HEADER(v); }

	class Predecessor: public PreIterator<Predecessor, Edge> {
	public:
		inline Predecessor(const BackwardCFGAdapter& g, const Vertex& v): iter(v->outs()) { }
		inline bool ended (void) const { return iter.ended(); }
		const Edge item (void) const { return iter.item(); }
		void next(void) { iter++; }
	private:
		Block::EdgeIter iter;
	};

	class Iterator: public CFG::BlockIter {
	public:
		inline Iterator(const BackwardCFGAdapter& cfga): CFG::BlockIter(cfga.cfg->blocks()) { }
	};

	class Successor: public PreIterator<Successor, Edge> {
	public:
		inline Successor(const BackwardCFGAdapter& g, const Vertex& v): iter(v->ins()) { }
		inline bool ended (void) const { return iter.ended(); }
		const Edge item (void) const { return iter.item(); }
		void next(void) { iter++; }
	private:
		Block::EdgeIter iter;
	};

private:
	CFG *cfg;
};

}	//otawa


#endif /* OTAWA_CFG_CFGADAPTER_H_ */
