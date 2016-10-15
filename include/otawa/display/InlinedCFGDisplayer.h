/*
 *	display::InlinedCFG class interface
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
#ifndef OTAWA_DISPLAY_INLINEDCFGDISPLAYER_H_
#define OTAWA_DISPLAY_INLINEDCFGDISPLAYER_H_

#include <otawa/cfg.h>
#include <otawa/display/CFGDisplayer.h>

namespace otawa { namespace display {

class InlinedCFG: public AbstractGraph {
	friend class InlinedCFGDecorator;
	friend class InlinedCFGBlockIter;
	friend class InlinedCFGEdgeIter;

	class Vertex: public AbstractGraph::Vertex {
	public:
		inline Vertex(Block *b = 0): block(b) { }
		Block *block;
	};

	class Edge: public AbstractGraph::Edge {
	public:
		inline Edge(otawa::Edge *e = 0, otawa::Block* s = 0, otawa::Block* t = 0): edge(e), source(s), target(t) { }
		otawa::Edge *edge;		// when 0, that means the artificial edge
		otawa::Block *source; // only used for artificial edge
		otawa::Block *target; // only use for artificial edge
	};

	class InlinedCFGBlockIter: public datastruct::IteratorInst<const AbstractGraph::Vertex *> {
	public:
		inline InlinedCFGBlockIter(const CFG::BlockIter& iter): i(iter), stop(false) { }
		virtual bool ended(void) const;
		virtual const AbstractGraph::Vertex *item(void) const;
		virtual void next(void);
	private:
		CFG::BlockIter i;				// the current iterator
		genstruct::SLList<CFG::BlockIter> vbi;		// the stack of the return target
		bool stop;						// true when no more block to go through
		genstruct::SLList<CFG*> travledCFG;		// use to make sure that there can only be one instance of each CFG (two calls of a CFG V will goes to the same CFG)
	};

	class InlinedCFGEdgeIter: public datastruct::IteratorInst<const AbstractGraph::Edge *> {
	public:
		InlinedCFGEdgeIter(const Block::EdgeIter& iter, Block* b);
		virtual bool ended(void) const;
		virtual const AbstractGraph::Edge *item(void) const;
		virtual void next(void);

	private:
		Block::EdgeIter i;
		Block* sourceBlock; // the source of the edge
		Vector<InlinedCFG::Edge> artificialEdges;
		InlinedCFG::Edge currentArtificialEdge;
		bool moreArtificialEdges;	// used by ended()
	};

public:
	InlinedCFG(CFG& cfg);
	~InlinedCFG(void);

	// the pure virtual functions
	virtual datastruct::IteratorInst<const AbstractGraph::Vertex *> *vertices(void) const;
	virtual datastruct::IteratorInst<const AbstractGraph::Edge *> *outs(const AbstractGraph::Vertex& v) const;
	virtual datastruct::IteratorInst<const AbstractGraph::Edge *> *ins(const AbstractGraph::Vertex& v) const;
	virtual const AbstractGraph::Vertex& sourceOf(const AbstractGraph::Edge& v) const;
	virtual const AbstractGraph::Vertex& sinkOf(const AbstractGraph::Edge& v) const;
	virtual string id(const AbstractGraph::Vertex& v) const;

private:
	static CFG *cfg(const AbstractGraph& g) { return static_cast<const InlinedCFG&>(g)._cfg; }
	static Block *block(const AbstractGraph::Vertex& v) { return static_cast<const Vertex&>(v).block; }
	static otawa::Edge *edge(const AbstractGraph::Edge& v) { return static_cast<const Edge&>(v).edge; }

	CFG *_cfg;
	static Identifier<Vertex> VERTEX;
	static Identifier<Edge> EDGE;
};

class InlinedCFGDecorator: public display::CFGDecorator {
public:

	InlinedCFGDecorator(WorkSpace *ws);

	virtual void decorate(const AbstractGraph& graph, Text& caption, GraphStyle& style) const;
	virtual void decorate(const AbstractGraph& graph, const AbstractGraph::Vertex& vertex, Text& content, VertexStyle& style) const;
	virtual void decorate(const AbstractGraph& graph, const AbstractGraph::Edge& edge, Text& label, EdgeStyle& style) const;

protected:
	virtual void displayEndBlock(CFG *graph, Block *block, Text& content, VertexStyle& style) const;
	virtual void displayHeader(CFG *graph, BasicBlock *block, Text& content) const;

	inline WorkSpace *workspace(void) const { return ws; }

private:
	void decorate(CFG *graph, otawa::Block *source, otawa::Block *target, Text& label, EdgeStyle& style) const;
	WorkSpace *ws;
};


} }		// otawa::display

#endif /* INCLUDE_OTAWA_DISPLAY_CFGDISPLAYER_H_ */
