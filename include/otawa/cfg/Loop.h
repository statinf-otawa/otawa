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

#include <functional>

#include <elm/data/List.h>
#include <elm/data/Range.h>
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
	Address address() const;
	inline Block *header(void) const { return _h; }
	inline bool isTop(void) const { return _h == _h->cfg()->entry(); }
	inline Loop *parent(void) const { return _p; }
	inline CFG *cfg(void) const { return _h->cfg(); }
	inline bool isLeaf() const { return _c.isEmpty(); }

	inline int depth(void) const { return _d; }
	bool includes(Loop *il) const;
	inline bool equals(Loop *l) const { return l == this; }

	inline const List<Loop *>& subLoops() const { return _c; }

	inline const Vector<Edge *>& exitEdges() const { return **EXIT_LIST(_h); }

	class BlockIter: public PreIterator<BlockIter, Block *> {
	public:
		BlockIter();
		BlockIter(const Loop *loop);
		inline bool ended(void) const { return !_todo.isEmpty(); }
		inline Block *item(void) const { return _todo.first(); }
		void next(void);
	private:
		const Loop *_loop;
		List<Block *> _todo;
		dfa::BitSet _done;
	};

	class BlockRange {
	public:
		inline BlockRange(const Loop *loop): l(loop) { }
		inline BlockIter begin() const { return BlockIter(l); }
		inline BlockIter end() const { return BlockIter(); }
	private:
		const Loop *l;
	};

	inline BlockRange blocks() const { return BlockRange(this); }

	void forSubLoops(std::function<void(Loop *)> f);
	inline static void forSubLoops(CFG *g, std::function<void(Loop *)> f)
		{ top(g)->forSubLoops(f); }
	inline static void forSubLoops(WorkSpace *ws, std::function<void(Loop *)> f)
		{ for(auto g: *CFGCollection::get(ws)) forSubLoops(g, f); }

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
