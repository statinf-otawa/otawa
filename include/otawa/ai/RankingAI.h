/*
 *	OrderedAI class interface
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
#ifndef INCLUDE_OTAWA_AI_ORDERED_AI_H_
#define INCLUDE_OTAWA_AI_ORDERED_AI_H_

#include <elm/data/SortedList.h>
#include <elm/types.h>
#include <otawa/prop/Identifier.h>
#include "features.h"

namespace otawa { namespace ai {

using namespace elm;

template <class A, class R = PropertyRanking>
class RankingAI {
public:
	typedef A adapter_t;
	typedef typename A::domain_t domain_t;
	typedef typename domain_t::t t;
	typedef typename A::graph_t graph_t;
	typedef typename graph_t::vertex_t vertex_t;
	typedef typename A::store_t store_t;

	RankingAI(A& adapter, R& rank = single<R>()):
		_adapter(adapter),
		_rank(rank)
		{ }

	void run(void) {
		t s;
		_todo.add(_adapter.graph().entry());
		while(_todo) {

			// process current item
			vertex_t v = _todo.first();
			_todo.removeFirst();
			_adapter.update(v, s);

			// propagate modification
			t p = _adapter.store().get(v);
			if(!_adapter.domain().equals(s, p)) {
				_adapter.store().set(v, s);
				for(auto v_w = _adapter.graph().succs(v); v_w; v_w++)
					_todo.add(_adapter.graph().sinkOf(v_w));
			}
		}
	}

	inline int doCompare(vertex_t v1, vertex_t v2) const { return _rank.rankOf(v1) - _rank.rankOf(v2); }

private:
	A& _adapter;
	R& _rank;
	SortedList<vertex_t, RankingAI<A, R>> _todo;
};

} }		// otawa::ai

#endif /* INCLUDE_OTAWA_AI_RANKING_AI_H_ */

