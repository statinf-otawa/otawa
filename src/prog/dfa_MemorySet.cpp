/*
 *	dfa::MemorySet class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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

#include <elm/util/misc.h>
#include <otawa/dfa/MemorySet.h>

namespace otawa { namespace dfa {

/**
 * @class MemorySet
 * This class is useful to represent sets of used memory addresses.
 * Memory addresses are handled as memory area that can added
 * or removed from the set. The set is also designed to support
 * efficient join operation (union or intersection).
 *
 * In addition, its memory management policy may be customized
 * by overloading this class and the methods allocate()
 * and reuse().
 *
 * This class is a controller class: it provides method to create
 * and handle memory set. The memory set itself is of type
 * MemorySet::t.
 *
 * @ingroup dfa
 */

#ifndef NDEBUG
#	define DO_CHECK
#endif
#ifndef DO_CHECK
#	define PRECOND(c)
#	define POSTCOND(x, c)	(x)
#else

	// 		forall a in m -> next(a) = null \/ a.top < next(a).address
	// /\	forall a in m -> a.address < a.top
	bool ordered(MemorySet::t m) {
		MemorySet::Iter i(m);
		if(i()) {
			MemArea prev = *i;
			if(prev.size() == 0)
				return false;
			for(i++; i(); i++) {
				if((*i).size() == 0
				|| prev.topAddress() >= (*i).address())
					return false;
				prev = *i;
			}
		}
		return true;
	}

#	define FATAL(f, l, m)	{ cerr << "FATAL:" << f << ":" << l << ": " << m << io::endl; elm::crash(); }
	template <class T>
	T check(T r, bool c, cstring f, int l, cstring m) {
		if(!c)
			FATAL(f, l, m)
		return r;
	}

#	define PRECOND(c)		if(!(c)) FATAL(__FILE__, __LINE__, #c);
#	define POSTCOND(x, c)	check(x, c, __FILE__, __LINE__, #c)
#endif


/**
 */
MemorySet::~MemorySet(void) {
}


/**
 * Static value representing a memory set.
 */
MemorySet::t const MemorySet::empty(0);


/**
 * Add an area to the set.
 * @param mem	Memory to add to.
 * @param area	Area to add.
 * @return		New memory with area added.
 */
MemorySet::t MemorySet::add(t mem, MemArea area) {
	PRECOND(ordered(mem));
	ASSERT(!area.isNull());
	// ASSERT(area.size() > 1); // comment out because for byte operation, the size is 1.
	node_t *m = mem;
	node_t *r = empty.h;
	node_t **l = &r;

	// copy head
	while(m && m->topAddress() < area.address()) {
		*l = allocate(m->area);
		l = &((*l)->next);
		m = m->next;
	}

	// insert isolated area
	if(!m || area.topAddress() < m->address()) {
		// m = null \/ a.addr <= m.top /\ a.top < m.addr
		*l = allocate(area);
		l = &((*l)->next);
	}

	// merge area
	else {
		// m.top >= a.addr /\ a.top >= m.addr
		Address base = min(m->address(), area.address());
		Address top = area.topAddress();
		while(m && m->address() <= area.topAddress()) {
			top = max(top, m->area.topAddress());
			m = m->next;
		}
		*l = allocate(MemArea(base, top));
		l = &((*l)->next);
	}

	// copy tail
	*l = reuse(m);
	return POSTCOND(r, ordered(r));
}


/**
 * Join two memories to include address of both memories.
 * @param m1	First memory.
 * @param m2	Second memory.
 * @return		Joined memory.
 */
MemorySet::t MemorySet::join(t m1, t m2) {
	PRECOND(ordered(m1));
	PRECOND(ordered(m2));

	// wipe out trivial cases
	if(m1 == empty)
		// m1 = [] -> m1 subset of r
		return reuse(m2);
	else if(m2 == empty)
		// m2 = [] -> m2 subset of r
		return reuse(m1);

	// start building the join
	node_t *r = empty;
	node_t **n = &r;
	node_t *p = m1, *q = m2;

	// merge in order
	while(p && q) {

		// prepare a new block
		Address base, top;
		if(p->address() < q->address()) {
			base = p->address();
			top = p->topAddress();
			p = p->next;
		}
		else {
			base = q->address();
			top = q->topAddress();
			q = q->next;
		}

		// merge joined blocks
		while(true) {
			if(p && p->address() <= top) {
				top = max(top, p->topAddress());
				p = p->next;
				continue;
			}
			if(q && q->address() <= top) {
				top = max(top, q->topAddress());
				q = q->next;
				continue;
			}
			break;
		}

		// build the new node
		*n = allocate(MemArea(base, top));
		n = &((*n)->next);
	}

	// add remaining queue
	*n = reuse(p ? p : q);
	return POSTCOND(r, ordered(r));
}


/**
 * Remove the given memory area from the memory set.
 * @param mem	Memory set to remove from.
 * @param area	Memory area to remove.
 * @return		mem without area removed.
 */
MemorySet::t MemorySet::remove(t mem, MemArea area) {
	PRECOND(ordered(mem));
	PRECOND(area.size() > 0);
	node_t *r = 0;
	node_t **n = &r;
	node_t *p = mem;

	while(p && p->address() <= area.topAddress()) {

		// block before: p.addr <= area.top /\ p.top <= area.addr
		if(p->topAddress() <= area.address()) {
			*n = allocate(p->area);
			n = &((*n)->next);
		}

		// current block is before area: p.addr <= area.top /\ p.top > area.addr
		else {

			if(area.address() <= p->address()) {
				if(p->topAddress() <= area.topAddress()) {
					// area.addr <= p.addr /\ p.top <= area.top -> p inside area
					p = p->next; // ignore the current block
					continue; // in case the area covers more than one blocks
				}
				else {
					// area.addr <= p.addr /\ area.top < p.top -> p at end of area
					*n = allocate(MemArea(area.topAddress(), p->topAddress()));
					n = &((*n)->next);
				}
			}

			else
				if(p->topAddress() <= area.topAddress()) {
					// p.addr < area.addr /\ p.top <= area.top -> p at beginning of area
					*n = allocate(MemArea(p->address(), area.address()));
					n = &((*n)->next);
				}
				else {
					// p.addr < area.addr /\ area.top < p.top -> area inside p
					*n = allocate(MemArea(p->address(), area.address()));
					n = &((*n)->next);
					*n = allocate(MemArea(area.topAddress(), p->topAddress()));
					n = &((*n)->next);
				}
		}

		p = p -> next;
	}

	*n = reuse(p);
	return POSTCOND(r, ordered(r));
}


/**
 * Test if the memory contains the given address.
 * @param t		Memory set to look in.
 * @param addr	Address to test.
 * @return		True if the address is in memory set, false else.
 */
bool MemorySet::contains(t mem, Address addr) {
	PRECOND(ordered(mem));
	ASSERT(!addr.isNull());
	node_t *p = mem;

	while(p && p->topAddress() <= addr)
		p = p->next;
	return p && p->area.contains(addr);
}


/**
 * Test if the memory contains (fully) the given memory
 * area.
 * @param mem	Memory to test.
 * @param area	Area to test.
 * @return		True if memory contains area, false else.
 */
bool MemorySet::contains(t mem, MemArea area) {
	PRECOND(ordered(mem));
	ASSERT(area);
	node_t *p = mem;

	// look in nodes
	while(p && p->address() <= area.address()) {
		if(p->area.includes(area))
			return true;
		p = p->next;
	}

	// no matching node found
	return false;
}


/**
 * Test if two memories are equal.
 * @param m1	First memory.
 * @param m2	Second memory.
 * @return		True if they equal, false else.
 */
bool MemorySet::equals(t m1, t m2) {
	PRECOND(ordered(m1));
	PRECOND(ordered(m2));
	node_t *p = m1, *q = m2;

	// INV: forall m in m1 \ p -> m in m2 \ q
	while(p && q) {
		if(p->area != q->area)
			// *p <> *q
			return false;
			// -> exists *p not m2 -> forall m in m1 -> m in m2 = _
		// *p = *q
		p = p->next;
		q =q ->next;
		// forall m in m1 \ p.next -> m in m2 \ q.next
	}

	// forall m in m1 \ p -> m in m2 \ q /\ (p = [] \/ q = [])
	if(p == q)
		// p = q = [];
		return true;
		// r = forall m in m1 \ [] -> m in m2 \ [] = forall m in m1 -> m in m2 = T
	else
		//		q = [] -> exist *p not in m2 -> (forall m in m1 -> m in m2) = _
		// \/	p = [] -> exist *q not in m1 -> (forall m in m1 -> m in m2) = _
		return false;
}


/**
 * Test if m1 is a subset of m2.
 * @param m1	Subset memory.
 * @param m2	Superset memory.
 * @return		True if m1 is a subset of m2.
 */
bool MemorySet::subset(t m1, t m2) {
	// PRE:		ordered(m1) /\ valid(m1) /\ ordered(m2) /\ valid(m2)
	// POST:	r = forall a in m1 -> exist b in m2 /\ a subset of b
	node_t *p = m1, *q = m2;

	// INV: forall a in m1 \ p -> exists b in m2 \ q /\ a subset of b /\ last(m1 \ p).top < q.addr
	// forall a in m1 \ m1 -> exists b in m2 \ m2 /\ a subset of b /\ last(m1 \ m1).top < q.addr
	while(p && q) {
		if(p->topAddress() < q->address())
			// A: p.top < q.addr -> forall a in q -> a not subset of *p
			p = p->next;
			// forall a in m1 \ p.next -> exists b in m2 \ q /\ a subset of b (by A) /\ last(m1 \ p.next).top = p.top < q.addr
		else if(p->address() <= q->address() && q->topAddress() <= p->topAddress())
			// A: *q in *p
			q = q->next;
			// forall a in m1 \ p -> exists b in m2 \ q.next /\ a subset of b (by INV /\ A) /\ last(m1 \ p).top < q.addr < q.next.addr (as ordered(m2))
		else
			// 		p.top >= q.addr /\ p.addr > q.addr	-> exist *q in m2 /\ *q not in m1 (by INV) => r = forall a in m1 -> exist b in m2 /\ a subset of b = _
			// \/	p.top >= q.addr /\ q.top > p.top	-> exist (q.addr, p.top) in m2 /\  (q.addr, p.top) not in m1 => r = forall a in m1 -> exist b in m2 /\ a subset of b = _
			return false;
	}

	// forall a in m1 \ p -> exists b in m2 \ q /\ a subset of b /\ (p = [] \/ q = [])
	if(!p)
		// p = []
		return true;
		// forall a in m1 -> exists b in m2 \ q /\ a subset of b => forall a in m1 -> exists b in m2 /\ a subset of b -> r = T
	else
		// p <> [] -> exists a in p /\ a not in m2
		return false;
		// -> exists a = *p in m1 /\ *p not in m2 -> r = _
}


/**
 * Free the given memory.
 * @param mem	Memory to free.
 */
void MemorySet::free(t mem) {
	node_t *m = mem;
	while(m) {
		node_t *n = m->next;
		delete m;
		m = n;
	}
}


/**
 * Allocate a new node.
 * @param area	Area to initialize the node.
 * @return		Allocated node.
 */
MemorySet::node_t *MemorySet::allocate(MemArea area) {
	node_t *r = new node_t;
	r->area = area;
	return r;
}


/**
 * Ensure that the given list of blocks is reused.
 * @param tail	Node list to re-use.
 * @return		Reusable version of list.
 */
MemorySet::node_t *MemorySet::reuse(node_t *tail) {
	PRECOND(ordered(tail));
	node_t *m = tail;
	node_t *r = 0;
	node_t **n = &r;
	while(m) {
		node_t *nm = allocate(m->area);
		*n = nm;
		n = &((*n)->next);
		m = m->next;
	}
	*n = 0;
	return POSTCOND(r, ordered(r));
}


/**
 *
 */
MemorySet::t MemorySet::meet(t m1, t m2) {
	PRECOND(ordered(m1));
	PRECOND(ordered(m2));
	node_t *p = m1, *q = m2;
	node_t *r = 0;
	node_t **n = &r;

#ifndef USE_OLD_MEMORY_SET_MEET
	for(p = m1; p; p = p->next) {
		// p                               |------|
		// q   |-------|     |---------|     |----------|
		//        q1              q2              q3
		// we ignore q1 and q2
		while(q && p->address() >= q->topAddress()) /* whole p after q */
			q = q->next;

		if(!q) // no more overlapping
			break;

		//         p1             p2
		// p    |------|      |---------|
		// q               |----------|
		// we ignore p1
		if(p->topAddress() <= q->address()) /* whole p before q */
			continue;

		// here p and q over-laps
		if(p->topAddress() > q->topAddress()) {
			while(q && p->topAddress() > q->topAddress()) {

				// p      |---------------------------------------------|
				// q   |-----|
				if(p->address() > q->address()) {
					*n = allocate(MemArea(p->address(), q->topAddress()));
					n = &((*n)->next);
				}
				// p      |---------------------------------------------|
				// q           |-----|
				else {
					*n = allocate(q->area);
					n = &((*n)->next);
				}
				q = q -> next;
			}

			// when reaching this position here
			// p   |--------------------------------|
			// q                                 |------|
			if(q && q->address() < p->topAddress()) {
				*n = allocate(MemArea(q->address(), p->topAddress()));
				n = &((*n)->next);
			}
		}
		else {
			// p      |-----------|
			// q   |------------------|
			if(p->address() > q->address()) {
				*n = allocate(p->area);
				n = &((*n)->next);
			}
			else {
				// p    |-----------|
				// q        |---------------|
				*n = allocate(MemArea(q->address(), p->topAddress()));
				n = &((*n)->next);
			}
		}
	}
#else
	while(p && q) {
		// ignore non-overlapping blocks
		// this does not work with
		// p     |-------------|
		// q  |---------------------|
		if(p->topAddress() < q->topAddress())
			p = p->next;
		// this does not work with
		// p     |-------------|
		// q         |-----|
		else if(q->topAddress() < p->topAddress())
			q = q->next;

		// overlapping blocks
		else {

			// select base node
			if(p->address() > q->address())
				swap(p, q);

			// consume intersecting nodes
			// this does not work with
			// p   |-------------|
			// q        |---|
			while(q && q->topAddress() <= p->topAddress()) {
				*n = allocate(q->area);
				n = &((*n)->next);
				q = q->next;
			}

			// last one
			if(q && q->address() < p->topAddress()) { // if(q->address() < p->topAddress()) {
				*n = allocate(MemArea(q->address(), p->topAddress()));
				n = &((*n)->next);
			}
			p = p->next;
		}
	}
#endif

	*n = 0;
	return POSTCOND(r, ordered(r));
}


/**
 */
io::Output& operator<<(io::Output& out, MemorySet::t mem) {
	bool f = true;
	out << "{ ";
	for(MemorySet::Iter m = mem.areas(); m(); m++) {
		if(f)
			f = false;
		else
			out << ", ";
		out << *m;
	}
	out << " }";
	return out;
}

} }		// otawa::dfa
