/*
 *	CFGGraph class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_AI_CFGGRAPH_H_
#define OTAWA_AI_CFGGRAPH_H_

#include <otawa/cfg.h>

namespace otawa { namespace ai {

/**
 * BiDiGraph implementation for CFG.
 */
class CFGGraph {
public:
	inline CFGGraph(CFG *cfg): _cfg(cfg) { }

	// BiDiGraph concept
	typedef Block *vertex_t;
	typedef Edge *edge_t;

	inline vertex_t entry(void) const { return _cfg->entry(); }
	inline vertex_t exit(void) const { return _cfg->exit(); }
	inline vertex_t sinkOf(edge_t e) const { return e->target(); }
	inline vertex_t sourceOf(edge_t e) const { return e->source(); }

	class Predecessor: public Block::EdgeIter {
	public:
		inline Predecessor(const CFGGraph& graph, vertex_t v): Block::EdgeIter(v->ins()) { }
	};

	class Successor: public Block::EdgeIter {
	public:
		inline Successor(const CFGGraph& graph, vertex_t v): Block::EdgeIter(v->outs()) { }
	};

	class Iterator: public CFG::BlockIter {
	public:
		inline Iterator(const CFGGraph& g): CFG::BlockIter(g._cfg->blocks()) { }
	};

	// Indexed concept
	inline int index(vertex_t v) const { return v->index(); }
	inline int count(void) const { return _cfg->count(); }

private:
	CFG *_cfg;
};

} }		// otawa::ai

#endif /* OTAWA_AI_CFGGRAPH_H_ */
