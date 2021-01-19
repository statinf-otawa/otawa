// TODO: need to mark the memory correctly!

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
#include <elm/array.h>
#include <elm/alloc/StackAllocator.h>
#include <otawa/hard/Platform.h>
#include "State.h"

namespace otawa { namespace dfa {

#define MEM_SIZE 64

using namespace elm;

template <class D, class T = StackAllocator>
class FastState {
public:
	typedef t::uint32 address_t;
	typedef t::uint32 register_t;
	typedef typename D::t value_t;
private:

	typedef struct node_t {
		// each node_t is constructed through the allocator, later, its member v is assigned from external values, which may require the memory
		// from the allocator as well. When GC is triggered due to the assignment of v, we need to ensure that the node_t will be marked.
		// Hence we are using the nodeAlloc (static member of the FastState, because node_t currently does not know its aggregate-parent class) to
		// keep the address of the node_t.
		inline node_t(address_t _a, const value_t& _v, node_t *_n = 0) {
			FastState<D,T>::nodeAlloc = this;
			a = _a;
			v = _v; // this could lead to memory allocation, and perhaps GC
			n = _n;
			FastState<D,T>::nodeAlloc = 0;
		}
		inline node_t(node_t *node) {
			FastState<D,T>::nodeAlloc = this;
			a = node->a;
			v = node->v; // this could lead to memory allocation, and perhaps GC
			n = 0;
			FastState<D,T>::nodeAlloc = 0;
		}
		address_t a;
		value_t v;
		node_t *n;
		inline void *operator new(size_t size, T& alloc) { return alloc.allocate(size); }
	} node_t;

	typedef struct state_t {
		//inline state_t(value_t **r, node_t *m): regs(r), mem(m) { }
		inline state_t(value_t **r, node_t *m[MEM_SIZE]): regs(r) {
			if(m != 0) {
				for(int i = 0; i < MEM_SIZE; i++)
					mems[i] = m[i];
			}
			else {
				for(int i = 0; i < MEM_SIZE; i++)
					mems[i] = 0;
			}
		}
		value_t **regs;
		//node_t *mem;
		node_t* mems[MEM_SIZE];
		inline void *operator new(size_t size, T& alloc) { return alloc.allocate(size); }
	} stat_t;

	static const int
		rblock_shift = 3,
		rblock_mask = (1 << rblock_shift) - 1,
		rblock_size = 1 << rblock_shift; 	// the number of registers per row
		// nrblock							// the number of rows of registers

	static node_t *nodeAlloc; // static because we want to collect the node, which is in our storage
	state_t *stateAlloc1;      // some operation requires 2 states as the input, this points to the first state
	state_t *stateAlloc2;      // some operation requires 2 states as the input, this points to the second state
	value_t **regsAlloc;       // the pointer to rows of registers
	value_t** regRowAlloc;     // the pointer to the current row of register
	value_t** regEachAlloc;    // the pointer to the current registers
	node_t *memAlloc;	       // the pointer to the current memory nodes that is currently in construction
	node_t** memEachRowAlloc;    // the pointer to the current registers
	value_t* tempValueAlloc;   // to hold a temporary created value
public:
	typedef state_t *t;



	/**
	 * Build a state.
	 * @param d		Domain manager.
	 * @param proc	Analyzed process.
	 * @param alloc	Stack allocator to use.
	 */
	inline FastState(D *d, dfa::State *state, T& alloc)
		: dom(d),
		  nrblock((state->process().platform()->regCount() + rblock_mask) >> rblock_shift),
		  allocator(alloc),
		  multi_max(8),
		  istate(state) {

			bot = make(true);
			top = make(false);

			regRowAlloc = new value_t*[nrblock];
			for(auto i = 0; i < nrblock; i++)
				regRowAlloc[i] = 0;

			int regCount = (nrblock* (1 << rblock_shift));
			regEachAlloc = new value_t*[regCount];
			for(auto i = 0; i < regCount; i++)
				regEachAlloc[i] = 0;

			tempValueAlloc = 0;
			regsAlloc = 0;
			memAlloc = 0;

			memEachRowAlloc = new node_t*[MEM_SIZE];
			for(int i = 0; i < MEM_SIZE; i++)
				memEachRowAlloc[i] = 0;

			stateAlloc1 = 0;
			stateAlloc2 = 0;
			nodeAlloc = 0;
	}

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
	const value_t& get(t s, int r) {
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
	t set(const t& s, int r, const value_t& v, bool show = false) {
		ASSERTP(r < nrblock * rblock_size, "register index out of bound");
		value_t temp = s->regs[r >> rblock_shift][r & rblock_mask];
		if(dom->equals(temp, v)) // if the existing value is the same as the value to assign, then no change
			return s;

		if(s == top && dom->equals(v, dom->top)) // assigning T to any register of the T state, will return T
			return top;

		stateAlloc1 = s;

		typename D::t **regs = allocator.template allocate<value_t *>(nrblock); // allocate the registers
		elm::array::copy(regs, s->regs, nrblock); // copy the pointer of each row to the just created registers
		regsAlloc = regs; // now the regs are initialized properly, track it

		//cerr << "DEBUG: set(" << r << ", "; dom->dump(cerr, v); cerr << ")\n";
		typename D::t *rblock = allocator.template allocate<typename D::t>(rblock_size); // the row to replace, however the individual registers are not
		regRowAlloc[0] = rblock;

		// we copy the content of each register in the row, note that the copying could result memory allocation
		// hence we need to track each register with regEachAlloc
		// elm::array::copy(rblock, s->regs[r >> rblock_shift], rblock_size);
		for(auto i = 0; i < rblock_size; i++) { // copy each register
			regEachAlloc[i] = new(&rblock[i]) value_t(s->regs[r >> rblock_shift][i]);
		}

		rblock[r & rblock_mask] = v; // set the value to the dedicated register, could lead GC, but we have everything tracked
		regs[r >> rblock_shift] = rblock; // only pointer assignment, no worry

		t res = new(allocator) state_t(regs, s->mems); // everything is tracked, no worry

		// clear the GC collection info
		stateAlloc1 = 0;
		regsAlloc = 0;
		regRowAlloc[0] = 0;
		for(auto i = 0; i < rblock_size; i++)
			regEachAlloc[i] = 0;

		if(dom->equals(v, dom->top) && equals(res, top)) // if the resulted state is T
			return top;

		return res; // NOITE: need to make sure the user of this function does not work with allocator, otherwise we need to protect res as well
	}


	/**
	 * Perform a store to memory.
	 * @param s		State to change.
	 * @param a		Address to store to.
	 * @param v		Value to store.
	 * @return		New state with store performed.
	 */
	t store(t s, address_t a, const value_t&  v) {



		stateAlloc1 = s;
		node_t *mem, *cur;
		node_t **pn = &mem;

		// check if it is necessary to have a new state_t, given a T state, assigning an address with a value of T, will maintain T
		if(s == top && dom->equals(v, dom->top))
			return top;

		// try to find the corresponding address in the state, if existed, then checking if the existing value and the assigned values are the same
		bool found = false;
		for(cur = s->mems[findMemsIndex(a)]; cur; cur = cur->n) { // find the address
			if(cur->a == a) {
				if(dom->equals(cur->v, v)) // if the value to assign is the same as the holding value, no need to allocate a new state
					return s;

				found = true;
				break;
			}
		}

		if(!found && dom->equals(v, dom->top)) // write bot/top to an address not in a record does not require a new state
			return s;

		// duplicating all the nodes whose memory address is smaller than the one to store.
		bool first = true; // use to mark the first node
		for(cur = s->mems[findMemsIndex(a)]; cur && cur->a < a; cur = cur->n) {
			*pn = new(allocator) node_t(cur);
			if(first) { // for the first allocated memory node, we associate memAlloc with the current mem head
				first = false;
				memAlloc = mem;
			}
			pn = &((*pn)->n);
		}

		// determine the next node for the node of designated address
		node_t *next;
		if(!cur) // if all the nodes in the current mem list are smaller then the address to store, hence nothing after the new node
			next = 0;
		else if(cur->a == a) // skipping the current node since its address is the same, the next node pointed by the current node will be used
			next = cur->n;
		else // if the current node's address is larger than the designated address, it will be used as the next node
			next = cur;

		// create the new node
		if(dom->equals(v, dom->top)) // even if the address is found, it is not necessary to store the Top value to the designated address, so skip
			*pn = next;
		else {
			*pn = new(allocator) node_t(a, v, next);
		}

		// build the new state

		node_t* mems[MEM_SIZE];
		for(int i = 0; i < MEM_SIZE; i++) {
			mems[i] = s->mems[i];
		}
		mems[findMemsIndex(a)] = mem;

		state_t* res = new(allocator) state_t(s->regs, mems);
		regsAlloc = 0;
		memAlloc = 0;
		stateAlloc1 = 0;
		stateAlloc2 = 0;
		return res;
	}

	/**
	 * Perform a load at the given address.
	 * @param s		Current state.
	 * @param a		Address to load from.
	 * @return		Load value.
	 */

	inline int findMemsIndex(address_t a) {
		elm::t::uint32 i = a;
		i = i >> 2; // get rid of the lowest 2 bits
		// i = i & 0x3F; // the lower 6 bits
		i = i & (MEM_SIZE - 1);
		return i;
	}


	const value_t& load(t s, address_t a) {
		if(s == bot)
			return dom->bot;

		// find the index of the mems
		int memsIndex = findMemsIndex(a);
		for(node_t *cur = s->mems[memsIndex]; cur; cur = cur->n)
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
		ASSERT(0); // see who uses this function, need to implement GC on it
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
		ASSERT(0); // wonder who comes here

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
			regs = allocator.template allocate<value_t *>(nrblock);
			for(int i = 0; i < nrblock; i++) {
				if(s1->regs[i] == s2->regs[i])
					regs[i] = s1->regs[i];
				else {
					regs[i] = allocator.template allocate<value_t>(rblock_size);
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
					regs = allocator.template allocate<value_t *>(nrblock);
					if(i > 0)
						array::copy(regs, s->regs, i);
				}
				regs[i] = allocator.template allocate<value_t>(rblock_size);
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
			for(auto i = 0; i < nrblock; i++)
				if(s1->regs[i] != s2->regs[i])
					for(auto j = 0; j < rblock_size; j++)
						if(!dom->equals(s1->regs[i][j], s2->regs[i][j]))
							return false;
		}

		// check memory
		for(int i = 0; i < MEM_SIZE; i++) {
			if(s1->mems[i] != s2->mems[i]) {
				node_t *c1, *c2;
				for(c1 = s1->mems[i], c2 = s2->mems[i]; c1 && c2; c1 = c1->n, c2 = c2->n) {
					if(c1 == c2)
						break;
					if(c1->a != c2->a || !dom->equals(c1->v, c2->v))
						return false;
				}
				if(c1 != c2)
					return false;
			}
		}
		// all is fine
		return true;
	}

	void print(io::Output& out, t s) {

		// display registers
		if(s == top) {
			out << "T";
			return;
		}
		else if(s == bot) {
			out << "⊥";
			return;
		}

		bool fst = true;
		for(auto i = 0; i < nrblock; i++)
			for(auto j = 0; j < rblock_size; j++)
				if(!dom->equals(s->regs[i][j], dom->bot) && !dom->equals(s->regs[i][j], dom->top)) {
					if(!fst)
						out << ", ";
					else
						fst = false;
					out << "r" << ((i * rblock_size) + j) << " = ";
					dom->dump(out, s->regs[i][j]);
				}

		// display memory
#ifdef SINGLE_MEM
		for(node_t *n = s->mem; n; n = n->n) {
			if(fst)
				fst = false;
			else
				out << ", ";
			out << Address(n->a) << " = ";
			dom->dump(out, n->v);
		}
#else
		// lets create a link list on the run....
		class AllMemList {
		public:
			node_t* memNode;
			AllMemList* next;
		};

		AllMemList* allMemList = 0;

		for(int i = 0; i < MEM_SIZE; i++) {
			node_t* currNodeT = s->mems[i];
			while(currNodeT) {
				// add the currNodeT into the list
				if(allMemList == 0) {
					allMemList = new AllMemList();
					allMemList->memNode = currNodeT;
					allMemList->next = 0;
				}
				else {
					AllMemList* curr = allMemList;
					AllMemList* prev = 0;
					while(curr) {
						if(curr->memNode->a > currNodeT->a) {
							AllMemList* temp = new AllMemList();
							temp->memNode = currNodeT;
							temp->next = curr;
							if(prev == 0) // first node
								allMemList = temp;
							else
								prev->next = temp;
							break;
						}
						else {
							prev = curr;
							curr = curr->next;
						}
					} // finishing searching all the node
					if(curr == 0) { // nodes' mem address are smaller than the currNodeT->a
						AllMemList* temp = new AllMemList();
						temp->memNode = currNodeT;
						prev->next = temp;
					}
				} // finish adding the currNodeT
				currNodeT = currNodeT->n;
			} // finishing adding the current mems[i]
		} // finishing adding all mems


		AllMemList* curr = allMemList;
		AllMemList* prev = 0;
		while(curr) {
			node_t *n = curr->memNode;
			if(fst)
				fst = false;
			else
				out << ", ";
			out << Address(n->a) << " = ";
			dom->dump(out, n->v);
			prev = curr;
			curr = curr->next;
			delete prev;
		}

		// print the contents of each mems[i]
		/*
		if(allMemList != 0) {
			elm::cout << io::endl;
			for(int i = 0; i < MEM_SIZE; i++) {
				node_t* n = s->mems[i];
				bool printing = false;
				bool fst = true;
				if(n) {
					elm::cout << "index = " << i << ":" << io::endl;
					printing = true;
				}
				while(n) {
					if(fst)
						fst = false;
					else
						out << ", ";
					out << Address(n->a) << " = ";
					dom->dump(out, n->v);
					n = n->n;
				}
				if(printing)
					elm::cout << io::endl;
			}
		}
		*/
#endif
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
	template <class W> t combine(const t& s1, const t& s2, W& w) {
		stateAlloc1 = s1;
		stateAlloc2 = s2;

		// we need to guard s1 and s2 as well

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
		if(s1->regs == s2->regs) // if two sets of registers are different, then we have to create new set
			regs = s1->regs;
		else {
			regs = allocator.template allocate<value_t *>(nrblock);
			regsAlloc = regs;

			for(auto i = 0; i < nrblock; i++) {
				if(s1->regs[i] == s2->regs[i])
					regs[i] = s1->regs[i];
				else {
					regs[i] = allocator.template allocate<value_t>(rblock_size);
					regRowAlloc[i] = regs[i];

					int currentJ = i << rblock_shift;
					for(auto j = 0; j < rblock_size; j++) {
						regEachAlloc[currentJ+j] = new(&regs[i][j])value_t(w.process(s1->regs[i][j], s2->regs[i][j]));
						value_t::tempPVAlloc = 0;
					}
				}
			}
		}

		// join memory
		node_t* mems[MEM_SIZE];
		for(int i = 0; i < MEM_SIZE; i++) {
			node_t *mem = 0, *cur1 = s1->mems[i], *cur2 = s2->mems[i];
			node_t **pn = &mem;
			bool first = true;
			while(cur1 != cur2 && cur1 && cur2) {

				// join the common address
				if(cur1->a == cur2->a) {
					value_t temp = w.process(cur1->v, cur2->v); // someone needs to protect the tab of the temp
					value_t::tempPVAlloc = 0;
					tempValueAlloc = &temp;
					// get rid of the bot/top states
					if(temp == dom->top) { }
					else if(temp == dom->bot) { }
					else {
						*pn = new(allocator) node_t(cur1->a, temp);
						if(first) {
							first = false;
							memAlloc = mem;
							memEachRowAlloc[i] = mem;
						}
						pn = &((*pn)->n); // pn now points to the address of the member n of the address node, so any new node_t will be pointed by n automatically
					}
					tempValueAlloc = 0;
					cur1 = cur1->n;
					cur2 = cur2->n;
				}

				// else join with T -> T, if the value for one address only presented for one state, the combine function will return Top for the address
				// TODO		We should take into account initialized memory!
				else if(cur1->a < cur2->a)
					cur1 = cur1->n;
				else
					cur2 = cur2->n;

			} // end of while cur1 != cur2
			if(cur1 == cur2)
				*pn = cur1;
			else
				*pn = 0;

			mems[i] = mem;
		}
		t res  = new(allocator) state_t(regs, mems);

		stateAlloc1 = 0;
		stateAlloc2 = 0;
		regsAlloc = 0;
		for(auto i = 0; i < nrblock; i++) {
			if(regRowAlloc[i]) {
				regRowAlloc[i] = 0;
				int maxj = (i << rblock_shift) + rblock_size;
				ASSERT(maxj < (istate->process().platform()->regCount() + rblock_size) ); // to make sure we don't have un-realistic maxj, which should cover the size of the regs of any given platform
				for(int j = i << rblock_shift; j < maxj; j++) {
					regEachAlloc[j] = 0;
				}
			}
		}
		memAlloc = 0;
		for(int i = 0; i < MEM_SIZE; i++)
			memEachRowAlloc[i] = 0;


		bool resultedTop = equals(res, top); // if the resulted state is top
		if(resultedTop)
			return top;

		// return join state
		return res;
	}


	void collect() {
		bool already = false;
		if(stateAlloc1) {
			collect(stateAlloc1);
		}
		if(stateAlloc2) {
			collect(stateAlloc2);
		}

		// then we have to mark the registers
		if(regsAlloc) {
			already = allocator.mark(regsAlloc, sizeof(value_t *)*nrblock);
			if(already) {
			}
			else {
				for(auto i = 0; i < nrblock; i++) { // for each row of registers
					if(regRowAlloc[i]) {
						already = allocator.mark(regRowAlloc[i], sizeof(value_t)*rblock_size);
						if(already) {
						}
						else {
							auto maxj = (i << rblock_shift) + rblock_size;
							ASSERT(maxj < (istate->process().platform()->regCount() + rblock_size) ); // to make sure we don't have un-realistic maxj, which should cover the size of the regs of any given platform
							for(auto j = i << rblock_shift; j < maxj; j++) { // now mark the domain for each register
								if(regEachAlloc[j])
									regEachAlloc[j]->collect(&allocator);
							} // end of each register
						} // end of registers to mark
					} // end of each row
				} // end of each row checking
			} // registers pack need to be marked
		}

		if(memAlloc) {
			for(node_t *n = memAlloc; n; n = n->n) {
				already = allocator.mark(n, sizeof(node_t));
				if(already) {
				}
				else {
					(n->v).collect(&allocator);
				}
			}
		}

		if(nodeAlloc) {
			already = allocator.mark(nodeAlloc, sizeof(node_t));
			if(already) {
			}
		}

		if(tempValueAlloc) {
			tempValueAlloc->collect(&allocator);
		}

		if(value_t::tempPVAlloc) {
			value_t::tempPVAlloc->collect(&allocator);
		}

		for(int i = 0; i < MEM_SIZE; i++) {
			for(node_t *n = memEachRowAlloc[i]; n; n = n->n) {
				already = allocator.mark(n, sizeof(node_t));
				if(already) {
				}
				else {
					(n->v).collect(&allocator);
				}
			}
		}



	}

	int collect(t _s, int currCount = 0, bool show=false) {
		bool already = false;
		// first mark the state

		already = allocator.mark(_s, sizeof(state_t));
		if(already) {
			return (currCount); // no need to collect the rest since they are already collected
		}

		// then we have to mark the registers
		already = allocator.mark(_s->regs, sizeof(value_t *)*nrblock);
		if(already) {
		}
		else {
			for(auto i = 0; i < nrblock; i++) {
				already = allocator.mark(_s->regs[i], sizeof(value_t)*rblock_size);
				if(already) {
				}
				else {
					// now mark the domain
					for(auto j = 0; j < rblock_size; j++) {
						_s->regs[i][j].collect(&allocator, show);
					}
				}
			}
		} // end of collecting registers

		// collecting memory

		for(int i = 0; i < MEM_SIZE; i++) {
			for(node_t *n = _s->mems[i]; n; n = n->n) {
				already = allocator.mark(n, sizeof(node_t));
				if(already) {
					break;
					// return (currCount+1);
				}
				else {
					(n->v).collect(&allocator);
				}
			} // end of the mem
		} // end of all mems

		return (currCount+1);
	}

private:

	/**
	 * Make a basic state.
	 * @param bot	True if it is a bottom state, false else.
	 * @return		Created state (must not be released by the user).
	 */
	t make(bool bot) {
		value_t **regs = allocator.template allocate<value_t *>(nrblock);
		for(auto i = 0; i < nrblock; i++)
			regs[i] = 0;
		regsAlloc = regs; // register the current regs to be collected

		for(auto i = 0; i < nrblock; i++) {
			regs[i] = allocator.template allocate<value_t>(rblock_size);
			for(auto j = 0; j < rblock_size; j++)
				if(bot) {
					//regs[i][j] = dom->bot;
					new(&regs[i][j])value_t(dom->bot);
					ASSERT(regs[i][j] == dom->bot);
				}
				else {
					//regs[i][j] = dom->top;
					new(&regs[i][j])value_t(dom->top);
					ASSERT(regs[i][j] == dom->top);
				}
		}
		t res = new(allocator) state_t(regs, 0);
		regsAlloc = 0; // de-register the current regs to be collected
		return res;
	}

	D *dom;
	int nrblock;
	T& allocator;
	int multi_max;
	dfa::State *istate;
public:
	t top, bot;

};

template <class D, class T> typename FastState<D, T>::node_t* FastState<D, T>::nodeAlloc = 0;
} }	// otawa::dfa

#endif /* OTAWA_DFA_FASTSTATE_H_ */
