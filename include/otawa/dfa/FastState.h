/*
 *	FastState class
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011-12, IRIT UPS.
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
#ifndef OTAWA_DFA_FASTSTATE_H_
#define OTAWA_DFA_FASTSTATE_H_

#include <elm/types.h>
#include <elm/util/array.h>
#include <elm/alloc/StackAllocator.h>
#include <otawa/hard/Platform.h>
#include "State.h"

namespace otawa { namespace dfa {

using namespace elm;

template <class D>
class FastState {
public:
	typedef t::uint32 address_t;
	typedef t::uint32 register_t;
	typedef typename D::t value_t;
private:

	typedef struct node_t {
		inline node_t(address_t _a, value_t _v, node_t *_n = 0): a(_a), v(_v), n(_n) { }
		inline node_t(node_t *node): a(node->a), v(node->v), n(0) { }
		address_t a;
		value_t v;
		node_t *n;
		inline void *operator new(size_t size, StackAllocator& alloc) { return alloc.allocate(size); }
	} node_t;

	typedef struct state_t {
		inline state_t(value_t **r, node_t *m): regs(r), mem(m) { }
		value_t **regs;
		node_t *mem;
		inline void *operator new(size_t size, StackAllocator& alloc) { return alloc.allocate(size); }
	} stat_t;

	static const elm::t::size
		rblock_shift = 3,
		rblock_mask = (1 << rblock_shift) - 1,
		rblock_size = 1 << rblock_shift;

public:
	typedef state_t *t;

	/**
	 * Build a state.
	 * @param d		Domain manager.
	 * @param proc	Analyzed process.
	 * @param alloc	Stack allocator to use.
	 */
	inline FastState(D *d, dfa::State *state, StackAllocator& alloc)
		: dom(d),
		  nrblock((state->process().platform()->regCount() + rblock_mask) >> rblock_shift),
		  allocator(alloc),
		  bot(make(true)), top(make(false)),
		  multi_max(8),
		  istate(state){ }

	/**
	 * Get the max number of multiple load/store before jumping to top.
	 * @return	Maximum number of effective load/store.
	 */
	inline int getMultiMax(void) const { return multi_max; }

	/**
	 * Set the maximum number of multiple load/store before approximating to top.
	 * @param new_multi_max		New maximum number of multiple load/store.
	 */
	inline void setMultiMax(int new_multi_max) { multi_max = new_multi_max; }

	/**
	 * Get register value.
	 * @param s	State to look at.
	 * @param r	Register to get.
	 * @return	Register value.
	 */
	value_t get(t s, register_t r) {
		ASSERTP(r < nrblock * rblock_size, "register index out of bound");
		return s->regs[r >> rblock_shift][r & rblock_mask];
	}

	/**
	 * Set a register value.
	 * @param s		State to change.
	 * @param r		Register to set.
	 * @param v		Value to set.
	 * @return		State after change.
	 */
	t set(t s, register_t r, value_t v) {
		ASSERTP(r < nrblock * rblock_size, "register index out of bound");
		//cerr << "DEBUG: set(" << r << ", "; dom->dump(cerr, v); cerr << ")\n";
		typename D::t *rblock = allocator.allocate<typename D::t>(rblock_size);
		elm::array::copy(rblock, s->regs[r >> rblock_shift], rblock_size);
		rblock[r & rblock_mask] = v;
		typename D::t **regs = allocator.allocate<value_t *>(nrblock);
		elm::array::copy(regs, s->regs, nrblock);
		regs[r >> rblock_shift] = rblock;
		t res = new(allocator) state_t(regs, s->mem);
		//cerr << "RESULT="; print(cerr, res);
		return res;
	}

	/**
	 * Perform a store to memory.
	 * @param s		State to change.
	 * @param a		Address to store to.
	 * @param v		Value to store.
	 * @return		New state with store performed.
	 */
	t store(t s, address_t a, value_t v) {
		node_t *mem, *cur;
		node_t **pn = &mem;

		// look for position
		for(cur = s->mem; cur && cur->a < a; cur = cur->n) {
			*pn = new(allocator) node_t(cur);
			pn = &((*pn)->n);
		}

		// determine the next node
		node_t *next;
		if(!cur)
			next = 0;
		else if(cur->a == a)
			next = cur->n;
		else
			next = cur;

		// create the new node
		if(v == dom->top)
			*pn = next;
		else
			*pn = new(allocator) node_t(a, v, next);

		// build the new state
		return new(allocator) state_t(s->regs, mem);
	}

	/**
	 * Perform a load at the given address.
	 * @param s		Current state.
	 * @param a		Address to load from.
	 * @return		Load value.
	 */
	value_t load(t s, address_t a) {
		if(s == bot)
			return dom->bot;
		for(node_t *cur = s->mem; cur; cur = cur->n)
			if(cur->a == a)
				return cur->v;
		return dom->top;
	}

	/**
	 * Perform a multiple-store (joining on all memories that are modified).
	 * @param s		State to store in.
	 * @param a		Start address of the area.
	 * @param b		Last address past the area.
	 * @param off	Offset between stored objects.
	 * @param v		Value to set.
	 * @return		New state.
	 */
	t store(t s, address_t a, address_t b, ot::size off, value_t v) {
		ASSERT(a < b);
		ASSERT(off > 0);

		// special cases
		if(s == bot)
			return s;
		node_t *mem = 0, *cur = 0;
		node_t **pn = &mem;

		// set all nodes of the area to top
		if((b - a) / off > multi_max || v == D::top) {
			for(cur = s->mem; cur && cur->a < b; cur = cur->n)
				if(cur->a != a) {
					if(a < cur->a)
						a += (cur->a - a + off - 1) / off * off;
					*pn = new(allocator) node_t(cur);
					pn = &((*pn)->n);
				}
		}

		// traverse the memory
		else {
			for(cur = s->mem; cur && cur->a < b; cur = cur->n)
				if(cur->a == a) {
					value_t v = dom->join(cur->v, v);
					a += off;
					if(v != dom->top) {
						*pn = new(allocator) node_t(cur->a, v);
						pn = &((*pn)->n);
					}
				}
				else if(a < cur->a)
					a += (cur->a - a + off - 1) / off * off;
				else {
					*pn = new(allocator) node_t(cur);
					pn = &((*pn)->n);
				}
		}

		// finalize the new state
		*pn = cur;
		s = new(allocator) state_t(s->regs, mem);
		return s;
	}

	/**
	 * Store to T address (set all memory to T).
	 * @param s		State to store to.
	 * @return		s with memory set to T.
	 */
	t storeAtTop(t s) {
		return new(allocator) state_t(s->regs, 0);
	}

	/**
	 * Load multiple values from a memory area.
	 * @param s		State to load from.
	 * @param a		First address of the area.
	 * @param b		Last address past the area.
	 * @param off	Offset of objects between the area.
	 * @return		Union of found objects.
	 */
	value_t load(t s, address_t a, address_t b, ot::size off) {
		ASSERT(a < b);
		ASSERT(off > 0);

		// special cases
		if((b - a) / off > multi_max)
			return dom->top;

		// load the data
		value_t r = dom->bot;
		for(node_t *cur = s->mem; cur && cur->a < b; cur = cur->n)
			if(cur->a == a) {
				r = dom->join(r, cur->v);
				a += off;
			}
			else if(a < cur->a)
				a += (cur->a - a + off - 1) / off * off;
		return r;
	}

	/**
	 * Perform join of both states.
	 */
	t join(t s1, t s2) {

		// special cases
		if(s1 == s2)
			return s1;
		else if(s1 == top || s2 == top)
			return top;
		else if(s1 == bot)
			return s2;
		else if(s2 == bot)
			return s1;

		// join registers
		value_t **regs;
		if(s1->regs == s2->regs)
			regs = s1->regs;
		else {
			regs = allocator.allocate<value_t *>(nrblock);
			for(int i = 0; i < nrblock; i++) {
				if(s1->regs[i] == s2->regs[i])
					regs[i] = s1->regs[i];
				else {
					regs[i] = allocator.allocate<value_t>(rblock_size);
					for(int j = 0; j < rblock_size; j++)
						regs[i][j] = dom->join(s1->regs[i][j], s2->regs[i][j]);
				}
			}
		}

		// join memory
		node_t *mem = 0, *cur1 = s1->mem, *cur2 = s2->mem;
		node_t **pn = &mem;
		while(cur1 != cur2 && cur1 && cur2) {
			if(cur1->a == cur2->a) {
				*pn = new(allocator) node_t(cur1->a, dom->join(cur1->v, cur2->v));
				pn = &(*pn)->n;
				cur1 = cur1->n;
				cur2 = cur2->n;
			}
			// TODO		We should take into account initialized memory!
			else if(cur1->a < cur2->a)
				cur1 = cur1->n;
			else
				cur2 = cur2->n;
		}
		if(cur1 == cur2)
			*pn = cur1;
		else
			*pn = 0;

		// return join state
		return new(allocator) state_t(regs, mem);
	}

	/**
	 * Apply a function to transform a state.
	 *
	 * The worker object must match the following concept:
	 *
	 * @code
	 * class Worker {
	 * public:
	 * 		value_t process(value_t v);
	 * };
	 * @endcode
	 *
	 * @param in	Input state.
	 * @param w		Worker object.
	 * @return		Output state.
	 */
	template <class W> t map(t s, W& w) {

		// register filtering
		value_t **regs = 0;
		value_t rblock[rblock_size];
		for(int i = 0; i < nrblock; i++) {

			// filter the block
			bool changed = false;
			for(int j = 0; j < rblock_size; j++) {
				rblock[j] = w.process(s->regs[i][j]);
				changed |= !dom->equals(rblock[j], s->regs[i][j]);
			}

			// need duplication?
			if(changed) {
				if(!regs) {
					regs = allocator.allocate<value_t *>(nrblock);
					if(i > 0)
						array::copy(regs, s->regs, i);
				}
				regs[i] = allocator.allocate<value_t>(rblock_size);
				array::copy(regs[i], rblock, rblock_size);
			}
			else if(regs)
				regs[i] = s->regs[i];
		}
		if(!regs)
			regs = s->regs;

		// memory filtering
		node_t *mem = s->mem;
		node_t *cur = s->mem, **pn = &mem, *last;
		for(cur = s->mem, last = s->mem; cur; cur = cur->n) {
			value_t v = w.process(cur->v);
			if(!dom->equals(cur->v, v)) {

				// copy the intermediate nods
				for(node_t *p = last; p != cur; p = p ->n) {
					*pn = new(allocator) node_t(p);
					pn = &((*pn)->n);
				}

				// add the changed node
				*pn = new(allocator) node_t(cur->a, v);
				last = cur->n;
			}
		}
		*pn = last;

		// return new state if required
		if(mem == s->mem && regs == s->regs)
			return s;
		else
			return new(allocator) state_t(regs, mem);
	}

	/**
	 * Test if both states are equal.
	 * @param s1	First state.
	 * @param s2	Second state.
	 * @return		True if they equal, false else.
	 */
	bool equals(t s1, t s2) {
		if(s1 == s2)
			return true;

		// check registers
		if(s1->regs != s2->regs) {
			for(int i = 0; i < nrblock; i++)
				if(s1->regs[i] != s2->regs[i])
					for(int j = 0; j < rblock_size; j++)
						if(!dom->equals(s1->regs[i][j], s2->regs[i][j]))
							return false;
		}

		// check memory
		if(s1->mem != s2->mem) {
			node_t *c1, *c2;
			for(c1 = s1->mem, c2 = s2->mem; c1 && c2; c1 = c1->n, c2 = c2->n) {
				if(c1 == c2)
					break;
				if(c1->a != c2->a || !dom->equals(c1->v, c2->v))
					return false;
			}
			if(c1 != c2)
				return false;
		}

		// all is fine
		return true;
	}

	void print(io::Output& out, t s) {

		// display registers
		out << '\t';
		bool fst = true;
		for(int i = 0; i < nrblock; i++)
			for(int j = 0; j < rblock_size; j++)
				if(!dom->equals(s->regs[i][j], dom->bot)) {
					if(!fst)
						out << ", ";
					else
						fst = false;
					out << "r" << ((i * rblock_size) + j) << " = ";
					dom->dump(out, s->regs[i][j]);
				}
		if(fst)
			out << "no register set";
		out << io::endl;

		// display memory
		out << '\t';
		fst = true;
		for(node_t *n = s->mem; n; n = n->n) {
			if(fst)
				fst = false;
			else
				out << ", ";
			out << Address(n->a) << " = ";
			dom->dump(out, n->v);
		}
		if(fst)
			out << "no memory set";
		out << io::endl;
	}

	/**
	 * Apply a function to combine two states. This function assumes
	 * that the applied operation is idempotent and monotonic and use it
	 * to speed up the combination.
	 *
	 * The worker object must match the following concept:
	 *
	 * @code
	 * class Worker {
	 * public:
	 * 		value_t process(value_t v1, value_t v2);
	 * };
	 * @endcode
	 *
	 * @param s1	First state.
	 * @param s2	Second state.
	 * @param w		Worker object.
	 * @return		Output state.
	 */
	template <class W> t combine(t s1, t s2, W& w) {

		// special cases
		if(s1 == s2)
			return s1;
		else if(s1 == top || s2 == top)
			return top;
		else if(s1 == bot)
			return s2;
		else if(s2 == bot)
			return s1;

		// join registers
		value_t **regs;
		if(s1->regs == s2->regs)
			regs = s1->regs;
		else {
			regs = allocator.allocate<value_t *>(nrblock);
			for(int i = 0; i < nrblock; i++) {
				if(s1->regs[i] == s2->regs[i])
					regs[i] = s1->regs[i];
				else {
					regs[i] = allocator.allocate<value_t>(rblock_size);
					for(int j = 0; j < rblock_size; j++)
						regs[i][j] = w.process(s1->regs[i][j], s2->regs[i][j]);
				}
			}
		}

		// join memory
		node_t *mem = 0, *cur1 = s1->mem, *cur2 = s2->mem;
		node_t **pn = &mem;
		while(cur1 != cur2 && cur1 && cur2) {

			// join the common address
			if(cur1->a == cur2->a) {
				*pn = new(allocator) node_t(cur1->a, w.process(cur1->v, cur2->v));
				pn = &((*pn)->n);
				cur1 = cur1->n;
				cur2 = cur2->n;
			}

			// else join with T -> T
			// TODO		We should take into account initialized memory!
			else if(cur1->a < cur2->a)
				cur1 = cur1->n;
			else
				cur2 = cur2->n;

		}
		if(cur1 == cur2)
			*pn = cur1;
		else
			*pn = 0;

		// return join state
		return new(allocator) state_t(regs, mem);
	}

private:

	/**
	 * Make a basic state.
	 * @param bot	True if it is a bottom state, false else.
	 * @return		Created state (must not be released by the user).
	 */
	t make(bool bot) {
		value_t **regs = allocator.allocate<value_t *>(nrblock);
		for(int i = 0; i < nrblock; i++) {
			regs[i] = allocator.allocate<value_t>(rblock_size);
			for(int j = 0; j < rblock_size; j++)
				regs[i][j] = dom->bot;
		}
		return new(allocator) state_t(regs, 0);
	}

	D *dom;
	elm::t::size nrblock;
	StackAllocator& allocator;
	int multi_max;
	dfa::State *istate;
public:
	t top, bot;

};

} }	// otawa::dfa

#endif /* OTAWA_DFA_FASTSTATE_H_ */
