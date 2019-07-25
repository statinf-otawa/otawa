/*
 *	ArrayStore class interface
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
#ifndef OTAWA_AI_ARRAYSTORE_H_
#define OTAWA_AI_ARRAYSTORE_H_

namespace otawa { namespace ai {

/**
 * Stockage of output values as an array.
 * @param D		Domain type.
 * @param G		Graph type.
 */
template <class D, class G>
class ArrayStore {
public:
	typedef D domain_t;
	typedef G graph_t;
	typedef typename D::t t;
	typedef typename G::vertex_t vertex_t;
	typedef typename G::edge_t edge_t;

	ArrayStore(D& dom, G& graph): _dom(dom), _graph(graph), map(new typename D::t[_graph.count()]) {
		clear();
	}

	void clear(void) {
		for(int i = 0; i < _graph.count(); i++)
			_dom.copy(map[i], _dom.bot());
	}

	inline void set(vertex_t v, t s) { map[_graph.index(v)] = s; }

	void set(edge_t e, t s) {
		int i = _graph.index(_graph.sourceOf(e));
		map[i] = _dom.join(map[i], s);
	}

	inline void reset(void) { clear(); }
	inline t get(vertex_t v) const { return map[_graph.index(v)]; }
	inline t get(edge_t e) const { return map[_graph.index(_graph.sourceOf(e))]; }
	inline D& domain(void) const { return _dom; }
	inline G& graph(void) const { return _graph; }

private:
	D& _dom;
	G& _graph;
	typename D::t *map;
};

} }		// otawa::ai

#endif /* INCLUDE_OTAWA_AI_ARRAYSTORE_H_ */
