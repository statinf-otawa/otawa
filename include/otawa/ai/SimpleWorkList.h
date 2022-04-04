/*
 *	WorkListDriver class interface
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
#ifndef OTAWA_AI_SIMPLEWORKLIST_H_
#define OTAWA_AI_SIMPLEWORKLIST_H_

#include <elm/data/ListQueue.h>
#include <elm/util/BitVector.h>
#include <otawa/cfg/features.h>

namespace otawa { namespace ai {

class SimpleWorkList {
public:
	inline SimpleWorkList(const CFGCollection *coll): in(coll->countBlocks()) { }

	inline bool isEmpty() const { return q.isEmpty(); }
	inline operator bool() const { return !isEmpty(); }

	inline void put(Block *v) {
		if(!in.bit(v->id())) {
			q.put(v);
			in.set(v->id());
		}
	}

	inline Block *get() {
		ASSERT(!q.isEmpty());
		auto v = q.get();
		in.clear(v->id());
		return v;
	}

private:
	ListQueue<Block *> q;
	BitVector in;
};

} }		// otawa::ai

#endif /* OTAWA_AI_SIMPLEWORKLIST_H_ */
