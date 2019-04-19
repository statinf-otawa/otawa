/*
 *	$Id$
 *	PreorderIterator class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_GRAPH_PREORDERITERATOR_H_
#define OTAWA_GRAPH_PREORDERITERATOR_H_

#include <elm/data/VectorQueue.h>

#include "../ograph/GenGraph.h"

namespace otawa { namespace ograph {

// PreorderIterator class
template <class G>
class PreorderIterator: public elm::PreIterator<PreorderIterator<G>, typename G::Vertex> {
public:

	PreorderIterator(const G& graph, typename G::Vertex entry)
	:	_graph(graph),
		visited(_graph.count()),
		queued(_graph.count())
	{
		queue.put(entry);
		queued.set(entry->index());
	}
	
	inline bool ended(void) const  { return queue.isEmpty(); }
	
	inline typename G::Vertex item(void) const { return queue.head(); }

	void next(void) {
		typename G::Vertex node = queue.get();
		visited.set(_graph.indexOf(node));
		for(typename G::OutIterator succ(_graph, node); succ(); succ++)
			if(!queued.bit(_graph.indexOf(succ->target()))) {
				ASSERT(!visited.bit(_graph.indexOf(succ->target())));
				bool check = true;
				for(typename G::InIterator pred(succ->target()); pred(); pred++) {
					check = visited.bit(_graph.indexOf(pred->source()));
					if(!check)
						break;
				}
				if(check) {
					queue.put(succ->target());
					queued.set(_graph.indexOf(succ->target()));
				}
			}
		queued.clear(node->index());			
	}

private:
	const G& _graph;
	elm::BitVector visited, queued;
	elm::VectorQueue<typename G::Vertex> queue;
};

} } // otawa::namespace

#endif /* OTAWA_GRAPH_PREORDERITERATOR_H_ */
