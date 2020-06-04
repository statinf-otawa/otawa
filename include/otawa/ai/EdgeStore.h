/*
 *	EdgeStore class interface
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
#ifndef OTAWA_AI_EDGESTORE_H_
#define OTAWA_AI_EDGESTORE_H_

#include <elm/data/HashMap.h>

namespace otawa { namespace ai {

using namespace elm;

/**
 * State storage on the edges.
 */
template <class D, class G>
class EdgeStore {
public:
	typedef D domain_t;
	typedef G graph_t;
	typedef typename D::t t;
	typedef typename G::vertex_t vertex_t;
	typedef typename G::edge_t edge_t;

	EdgeStore(D& dom, G& graph): _dom(dom), _graph(graph) {
	}

	void set(vertex_t v, t s) {
		for(typename G::Successor e(_graph, v); e(); e++)
			map.put(*e, s);
	}

	inline void set(edge_t e, const t& s) { map.put(e, s); }

	t get(vertex_t v) const {
		t s = _dom.bot();
		for(typename G::Successor e(_graph, v); e(); e++)
			_dom.join(s, map.get(e, _dom.bot()));
		return s;
	}

	inline const t& get(edge_t e) { return map.get(e, _dom.bot()); }

	inline D& domain(void) const { return _dom; }
	inline G& graph(void) const { return _graph; }

	void reset(void) {
		map.clear();
	}

private:
	D& _dom;
	G& _graph;
	HashMap<edge_t, t> map;
};

} }		// otawa::ai

#endif /* INCLUDE_OTAWA_AI_EDGESTORE_H_ */
