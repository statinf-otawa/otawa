/*
 *	$Id$
 *	BBStatCollector class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <otawa/cfg/features.h>
#include <otawa/stats/BBStatCollector.h>
#include <elm/util/BitVector.h>

namespace otawa {

// constant list management
/*template <class T>
class ListManager {
public:
	class Cons {
	public:
		inline const T& hd(void) const { return h; }
		inline Cons *tl(void) const { return t; }

	private:
		friend class ListManager<T>;
		inline Cons(void) { }
		void set(const T& hd, Cons *tl) { h = hd; t = tl; }
		void link(Cons *tl) { t = tl; }
		T h;
		Cons *t;
	};

	ListManager(int size = 1024): fst(0), free(0), _size(size) { }

	Cons *cons(const T& hd, Cons *tl) {
		Cons *res = 0;

		// look in free blocks
		if(free) {
			res = free;
			free = free->tl();
		}

		// take from chunk
		else if(fst && fst->used < _size) {
			res = &fst->t[fst->used];
			fst->used++;
		}

		// not enough memory
		else {

			// garbage collect
			garbage();

			// retry from free list
			if(free) {
				res = free;
				free = free->tl();
			}

			// new chunk
			else {
				chunk_t *chunk = (chunk_t *)(new char[sizeof(chunk_t) + sizeof(Cons *) * _size]);
				chunk->next = fst;
				chunk->used = 1;
				fst = chunk;
				res = &fst->t[0];
			}
		}

		// return result
		res->set(hd, tl);
		return res;
	}

protected:
	virtual void collect(void) = 0;

	void mark(Cons *item) {
		while(item) {
			chunk_t *chunk = find(item);
			int i = item - chunk->t;
			if(chunk->bits[word(i)] & mask(i));
				break;
			chunk->bits[word(i)] |= mask(i);
			item = item->tl();
		}
	}

private:
	typedef struct chunk_t {
		struct chunk_t *next;
		int used;
		t::uint32 *bits;
		Cons t[];
	} chunk_t;

	inline int words(void) const { return (_size + 31) / 32; }
	inline int word(int i) const { return i / 32; }
	inline int bit(int i) const { return i & 0x1f; }
	inline int mask(int i) const { return 1 << bit(i); }

	chunk_t *find(Cons *cons) {
		for(chunk_t *cur = fst; cur; cur = cur->next)
			if(cur->t <= cons && cons < cur->t + _size)
				return cur;
		ASSERTP(false, "bad Cons * pointer: something nasty happened");
		return 0;
	}

	void garbage(void) {

		// allocate bitvectors
		for(chunk_t *cur = fst; cur; cur = cur->next) {
			cur->bits = new t::uint32[words()];
			for(int i = 0; i < words(); i++)
				cur->bits[i] = 0;
		}

		// perform the garbage
		collect();

		// free bitvectors and collect unused cons
		for(chunk_t *cur = fst; cur; cur = cur->next) {
			for(int i = 0; i < _size; i++)
				if(!(cur->bits[word(i)] & mask(i))) {
					cur->t[i].link(free);
					free = &cur->t[i];
				}
			delete [] cur->bits;
		}
	}

	chunk_t *fst;
	Cons *free;
	int _size;
};*/


/*class ContextManager: public ListManager<ContextualStep> {
public:
	typedef Pair<BasicBlock *, Cons *> pair_t;
	typedef Vector<pair_t> todo_t;

	BasicBlock *bb;
	Cons *ctx;

	inline void push(void) {
		todo.push(pair_t(bb, ctx));
	}

	inline void pop(void) {
		pair_t pair = todo.pop();
		bb = pair.fst;
		ctx = pair.snd;
	}

	inline operator bool(void) const { return todo; }

protected:
	virtual void collect(void) {
		mark(ctx);
		for(int i = 0; i < todo.count(); i++)
			mark(todo[i].snd);
	}

private:
	todo_t todo;
};*/


/**
 * @class BBStatCollector;
 * This class alleviates the work of building a statistics collector.
 * It ensures the traversal of all basic blocks and calls collect() to let
 * the inheriting class do specific work for a basic block.
 */


/**
 * Build the BB statistics collector.
 * @param ws	Current workspace.
 */
BBStatCollector::BBStatCollector(WorkSpace *ws): _ws(ws), _cfg(0) {
}


/**
 */
void BBStatCollector::collect(Collector& collector) {
	const CFGCollection *coll = INVOLVED_CFGS(ws());
	for(int i = 0; i < coll->count(); i++) {
		_cfg = coll->get(i);
		process(collector);
	}
}


/**
 * Process basic block of the current CFG.
 * @param collector		Collector to use.
 */
void BBStatCollector::process(Collector& collector) {
	const CFGCollection *coll = INVOLVED_CFGS(_ws);
	ASSERT(coll);
	for(int i = 0; i < coll->count(); i++) {
		_cfg = coll->get(i);
		processCFG(collector, coll->get(i));
	}
}

void BBStatCollector::processCFG(Collector& collector, CFG *cfg) {
	BitVector marks(_cfg->countBB());
	typedef Pair<CFG *, Edge *> call_t;
	Vector<Edge *> todo;
	Vector<call_t> calls;

	// initialization
	calls.push(call_t(cfg, 0));
	//collector.enter(ContextualStep(ContextualStep::FUNCTION, cfg->address()));
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++)
		todo.push(edge);

	// traverse until the end
	while(todo) {
		Edge *edge = todo.pop();
		BasicBlock *bb;

		// null edge -> leaving function
		if(!edge) {
			edge = calls.top().snd;
			calls.pop();
			collector.leave();
			bb = edge->target();
		}

		// a virtual call ?
		else switch(edge->kind()) {

		case Edge::NONE:
			ASSERT(false);
			break;

		case Edge::VIRTUAL_RETURN:
		case Edge::CALL:
			bb = 0;
			break;

		case Edge::TAKEN:
		case Edge::NOT_TAKEN:
			if(!marks.bit(edge->target()->number()))
				bb = edge->target();
			else
				bb = 0;
			break;

		case Edge::VIRTUAL:
			if(edge->target() == cfg->exit()) {
				bb = 0;
				break;
			}

		case Edge::VIRTUAL_CALL: {
				bb = edge->target();

				// recursive call
				if(marks.bit(bb->number()))
					bb = 0;

				// simple call
				else {
					if(!edge->source()->isEntry())
						collector.enter(ContextualStep(ContextualStep::CALL, edge->source()->controlInst()->address()));
					collector.enter(ContextualStep(ContextualStep::FUNCTION, edge->target()->address()));
					BasicBlock *ret = VIRTUAL_RETURN_BLOCK(edge->source());
					if(!ret)
						ret = cfg->exit();
					CFG *called_cfg = CALLED_CFG(edge);
					if(!called_cfg)
						called_cfg = cfg;
					calls.push(call_t(called_cfg, BasicBlock::InIterator(ret)));
					todo.push(0);
				}
			}
			break;
		}

		// process basic block
		if(bb) {
			collect(collector, bb);
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				todo.push(edge);
			marks.set(bb->number());
		}
	}
}


/**
 * @fn void BBStatCollector::collect(Collector& collector, BasicBlock *bb);
 * This method is called for each basic block in the current workspace (except for
 * syntactic entry and exit blocks). It must specialized by the inheriting class
 * to provide specific work.
 *
 * The methods ws(), cfg() and path() allows to get the current information about
 * the processed blocks.
 *
 * @param collector		The invoker collector to pass statistics information.
 * @param bb			Current basic block.
 */

}	// otawa
