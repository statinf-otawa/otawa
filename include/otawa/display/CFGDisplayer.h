/*
 *	display::CFGDisplayer class interface
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
#ifndef OTAWA_DISPLAY_CFGDISPLAYER_H_
#define OTAWA_DISPLAY_CFGDISPLAYER_H_

#include "Displayer.h"
#include <otawa/cfg.h>

namespace otawa { namespace display {

class DisplayedCFG: public AbstractGraph {
	friend class CFGDecorator;
	friend class BlockIter;
	friend class EdgeIter;

	class Vertex: public AbstractGraph::Vertex {
	public:
		inline Vertex(Block *b = 0): block(b) { }
		Block *block;
	};

	class Edge: public AbstractGraph::Edge {
	public:
		inline Edge(otawa::Edge *e = 0): edge(e) { }
		otawa::Edge *edge;
	};


public:
	DisplayedCFG(CFG& cfg);
	~DisplayedCFG(void);

	virtual dyndata::AbstractIter<const AbstractGraph::Vertex *> *vertices(void) const;
	virtual dyndata::AbstractIter<const AbstractGraph::Edge *> *outs(const AbstractGraph::Vertex& v) const;
	virtual dyndata::AbstractIter<const AbstractGraph::Edge *> *ins(const AbstractGraph::Vertex& v) const;
	virtual const AbstractGraph::Vertex& sourceOf(const AbstractGraph::Edge& v) const;
	virtual const AbstractGraph::Vertex& sinkOf(const AbstractGraph::Edge& v) const;
	virtual string id(const AbstractGraph::Vertex& v) const;

private:
	static CFG *cfg(const AbstractGraph& g) { return static_cast<const DisplayedCFG&>(g)._cfg; }
	static Block *block(const AbstractGraph::Vertex& v) { return static_cast<const Vertex&>(v).block; }
	static otawa::Edge *edge(const AbstractGraph::Edge& v) { return static_cast<const Edge&>(v).edge; }

	CFG *_cfg;
	static Identifier<Vertex> VERTEX;
	static Identifier<Edge> EDGE;
};

class CFGDecorator: public Decorator {
public:

	CFGDecorator(WorkSpace *ws);

	bool display_assembly;
	bool display_source_line;
	bool display_props;
	Color source_color, label_color;

	virtual void decorate(CFG *graph, Text& caption, GraphStyle& style) const;
	virtual void decorate(CFG *graph, Block *block, Text& content, VertexStyle& style) const;
	virtual void decorate(CFG *graph, otawa::Edge *edge, Text& label, EdgeStyle& style) const;

	virtual void decorate(const AbstractGraph& graph, Text& caption, GraphStyle& style) const;
	virtual void decorate(const AbstractGraph& graph, const AbstractGraph::Vertex& vertex, Text& content, VertexStyle& style) const;
	virtual void decorate(const AbstractGraph& graph, const AbstractGraph::Edge& edge, Text& label, EdgeStyle& style) const;

	void setDisplayOptions(bool _da = true, bool _dsl = true, bool _dp = false, Color _sc = Color("darkgreen"), Color _lc = Color("blue"));

protected:
	virtual void displayEndBlock(CFG *graph, Block *block, Text& content, VertexStyle& style) const;
	virtual void displaySynthBlock(CFG *graph, SynthBlock *block, Text& content, VertexStyle& style) const;
	virtual void displayBasicBlock(CFG *graph, BasicBlock *block, Text& content, VertexStyle& style) const;
	virtual void displayHeader(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displayBody(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displayAssembly(CFG *graph, BasicBlock *block, Text& content) const;
	virtual void displayProps(CFG *graph, BasicBlock *block, Text& content) const;

	inline WorkSpace *workspace(void) const { return ws; }

private:
	WorkSpace *ws;
};

} }		// otawa::display

#endif /* INCLUDE_OTAWA_DISPLAY_CFGDISPLAYER_H_ */
