/*
 *	Loop class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef INCLUDE_OTAWA_CFG_LOOP_H_
#define INCLUDE_OTAWA_CFG_LOOP_H_

#include <elm/data/List.h>
#include <otawa/dfa/BitSet.h>
#include "features.h"

namespace otawa {

class Loop: public PropList {
public:
	static Loop *of(Block *b);
	static Loop *top(CFG *cfg);
	static bool isBack(otawa::Edge *e) { return BACK_EDGE(e); }
	static bool isExit(otawa::Edge *e) { return LOOP_EXIT_EDGE(e); }
	static bool isHeader(Block *b) { return LOOP_HEADER(b); }

	Loop(Block *h);
	Loop(CFG *cfg);
	inline Block *header(void) const { return _h; }
	inline bool isTop(void) const { return !_h; }
	inline Loop *parent(void) const { return _p; }
	inline CFG *cfg(void) const { return _h->cfg(); }

	inline int depth(void) const { return _d; }
	bool includes(Loop *il) const;

	typedef List<Loop *>::Iter ChildIter;
	inline ChildIter subLoops(void) const { return ChildIter(_c); }
	inline ChildIter endSubLoops(void) const { return ChildIter(); }

	typedef Vector<Edge *>::Iter ExitIter;
	inline ExitIter exitEdges(void) const { return **EXIT_LIST(_h); }
	inline ExitIter endExitEdges(void) const { return *Vector<Edge *>::null; }

	class BlockIter: public PreIterator<BlockIter, Block *> {
	public:
		BlockIter(Loop *loop);
		inline bool ended(void) const { return !_todo.isEmpty(); }
		inline Block *item(void) const { return _todo.first(); }
		void next(void);
	private:
		Loop *_loop;
		List<Block *> _todo;
		dfa::BitSet _done;
	};

	static p::id<Loop *> ID;
private:
	Block *_h;
	Loop *_p;
	int _d;
	List<Loop *> _c;
};

io::Output& operator<<(io::Output& out, const Loop *l);

}	// otawa

#endif /* INCLUDE_OTAWA_CFG_LOOP_H_ */
