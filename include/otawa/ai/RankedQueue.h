/*
 *	RankedQueue class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2020, IRIT UPS.
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
#ifndef OTAWA_AI_RANKEDQUEUE_H_
#define OTAWA_AI_RANKEDQUEUE_H_

#include <otawa/cfg/features.h>
#include <otawa/ai/FlowAwareRanking.h>
#include <elm/data/SortedList.h>
#include <elm/util/BitVector.h>

#include <elm/data/ListQueue.h>
#include <elm/data/BinomialQueue.h>

#include <otawa/proc/Registration.h>

namespace otawa { namespace ai {

using namespace elm;

class Queue {

	class RankComparator {
	public:
		typedef Block *t;
		inline int doCompare(Block *b1, Block *b2) const {
			return RANK_OF(b1) - RANK_OF(b2);
		}
	};

public:

	static Requirement requirements;

	Queue(const CFGCollection *collection, int shift = 4):
		bs(collection->countBlocks()) { }

	inline bool isEmpty() const { return q.isEmpty(); }
	inline operator bool() const { return !isEmpty(); }

	void put(Block *b) {
		if(!bs.bit(b->id())) {
			bs.set(b->id());
			q.put(b);
		}
	}

	Block *get() {
		auto b = q.get();
		bs.clear(b->id());
		return b;
	}

private:
	BinomialQueue<Block *, RankComparator> q;
	BitVector bs;
};
Requirement Queue::requirements = p::require(otawa::ai::RANKING_FEATURE);

} }		// otawa::ai

#endif /* OTAWA_AI_RANKEDQUEUE_H_ */
