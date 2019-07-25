/*
 *	dfa::MemorySet class interface
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
#ifndef OTAWA_DFA_MEMORYSET_HPP_
#define OTAWA_DFA_MEMORYSET_HPP_

#include <elm/io.h>
#include <elm/PreIterator.h>
#include <otawa/base.h>

namespace otawa { namespace dfa {

class MemorySet {
	typedef struct node_t {
		struct node_t *next;
		MemArea area;
		inline Address address(void) const { return area.address(); }
		inline Address topAddress(void) const { return area.topAddress(); }
	} node_t;

public:

	class Iter: public PreIterator<Iter, MemArea> {
	public:
		inline Iter(node_t *set): p(set) { }
		inline bool ended(void) const { return !p; }
		inline const MemArea& item(void) const { return p->area; }
		inline void next(void) { p = p->next; }
	private:
		node_t *p;
	};

	typedef struct t {
		node_t * h;
		inline t(node_t *node) { h = node; }
		inline operator node_t *(void) const { return h; }
		inline Iter areas(void) const { return Iter(h); }
	} t;
	virtual ~MemorySet(void);

	// constructors
	static const t empty;
	virtual void free(t mem);
	t add(t mem, MemArea area);
	t remove(t mem, MemArea area);
	t join(t m1, t m2);
	t meet(t m1, t m2);

	// accessors
	static bool contains(t mem, Address addr);
	static bool contains(t mem, MemArea addr);
	static bool meets(t mem, MemArea area);
	static bool equals(t m1, t m2);
	static bool subset(t m1, t m2);

protected:
	virtual node_t *allocate(MemArea area);
	virtual node_t *reuse(MemorySet::node_t *tail);
};

inline bool operator==(MemorySet::t m1, MemorySet::t m2) { return MemorySet::equals(m1, m2); }
inline bool operator!=(MemorySet::t m1, MemorySet::t m2) { return !MemorySet::equals(m1, m2); }
inline bool operator<=(MemorySet::t m1, MemorySet::t m2) { return MemorySet::subset(m1, m2); }
inline bool operator<(MemorySet::t m1, MemorySet::t m2) { return MemorySet::subset(m1, m2) && !MemorySet::equals(m1, m2); }
inline bool operator>=(MemorySet::t m1, MemorySet::t m2) { return MemorySet::subset(m2, m1); }
inline bool operator>(MemorySet::t m1, MemorySet::t m2) { return MemorySet::subset(m2, m1) && !MemorySet::equals(m1, m2); }
inline bool operator%(Address a, MemorySet::t m) { return MemorySet::contains(m, a); }
inline bool operator%(MemArea a, MemorySet::t m) { return MemorySet::contains(m, a); }

io::Output& operator<<(io::Output& out, MemorySet::t m);

} }		// otawa::dfa

#endif /* OTAWA_DFA_MEMORYSET_HPP_ */
