/*
 *	icat3::PERSDomain class interface
 *	Copyright (c) 2016, IRIT UPS.
 *
 *	This file is part of OTAWA
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


#include "MustPersDomain.h"

namespace otawa { namespace icat3 {

/**
 * @class PersDomain
 * Implement the PERS analysis of L1 instruction cache according to:
 *
 * C. Ballabriga, H. Cassé. Improving the First-Miss Computation in
 * Set-Associative Instruction Caches.
 * In Euromicro Conference on Real-Time Systems (ECRTS 2008), IEEE, p.341-350, july 2008.
 *
 * @ingroup icat3
 */

/**
 * Build a PERS domain for PERS analysis.
 * @param coll	Collection of l-blocks.
 * @param set	Current cache set.
 * @param init	Initial ACS value (default to top).
 */
PersDomain::PersDomain(const LBlockCollection& coll, int set, const ACS *init)
:	n(coll[set].count()),
	_bot(),
	_top(coll[set].count(), BOT_AGE),
	_set(set),
	_coll(coll),
	A(coll.cache()->wayCount()),
	_init(init ? ACSStack(*init) : _top),
	tmp(coll[set].count())
{ }

/**
 * Print the given ACS.
 * @param a		ACS to print.
 * @param out	Output to use.
 */
void PersDomain::print(const t& a, io::Output& out) const {
	a.print(_set, _coll, out);
}

/**
 * Join 2 ACS.
 * @param d		ACS to join in (the target).
 * @param s		ACS to join with.
 */
void PersDomain::join(ACS& d, const ACS& s) {

	// J(a, a') = a" s.t. ∀ b ∈ B_s
	for (int b = 0; b < n; b++)

		// a"[b] = max(a[b], a'[b])	if a[b] ≠ ⊥ ∧ a'[b] ≠ ⊥
		if(d[b] != BOT_AGE && s[b] != BOT_AGE)
			d[b] = max(d[b], s[b]);

		// a"[b] = a'[b]			if a[b] = ⊥
		else if(d[b] == BOT_AGE)
			d[b] = s[b];

		// a"[b] = a[b]			else
}

/**
 * Update the given ACS according to a fetch
 * to given cache block.
 * @param lb	Fetched block.
 * @param a		ACS to update.
 */
void PersDomain::fetch(LBlock *lb, ACS& a) {
	int b = lb->index();
	// U_p(a, b) = a' s.t. ∀ b' ∈ B_s

	// if a[b] ∈  [0, A-1]
	if(0 <= a[b] && a[b] < A) {
		for(int b_ = 0; b_ < n; b_++)
			//a'[b'] = a[b'] + 1	if a[b'] < a[b] ∧ a[b'] ∉ { ⊥,  A }
			if(a[b_] < a[b] && a[b_] != BOT_AGE && a[b_] != A)
				a[b_] = a[b_] + 1;
			//a'[b'] = a[b']		else
	}

	// else
	else
		for(int b_ = 0; b_ < n; b_++)
		// a'[b'] = a[b'] + 1	if a[b'] ∉ { ⊥,  A }
			if(a[b_] != BOT_AGE && a[b_] != A)
				a[b_] = a[b_] + 1;
		// a'[b'] = a[b']		else

	//	a'[b'] = 0			if b' = b
	a[b] = 0;

}


/**
 * Update a single ACS.
 * @param o		Access operation to update with.
 * @param a		ACS to update.
 */
void PersDomain::update(const icache::Access& o, ACS& a) {
	switch(o.kind()) {
	case icache::NONE:
		ASSERT(false);
		break;
	case icache::FETCH:
		if(_coll.cache()->set(o.address()) == _set)
			fetch(LBLOCK(o), a);
		break;
	case icache::PREFETCH:
		if(_coll.cache()->set(o.address()) == _set) {
			tmp = a;
			fetch(LBLOCK(o), a);
			join(a, tmp);
		}
		break;
	}
}

/**
 * Update an ACS stack.
 * @param d		Target ACS stack.
 * @param s		Source ACS stack.
 */
void PersDomain::join(t& d, const t& s) {
	// J_mp(d, s) =

	// d		if s = ⊥
	if(s.isBottom())
		return;

	// s		if d = ⊥
	else if(d.isBottom())
		d = s;

	// a" s.t. ∀ i ∈ [0, n[, a"[i] = J_p(a[i], a'[i])
	else {
		for(int i = 0; i < min(d.depth(), s.depth()); i++)
			join(d[i], s[i]);
		join(d.whole(), s.whole());
		if(d.depth() < s.depth()) {
			for(int i = d.depth(); i < s.depth(); i++) {
				enter(d);
				d[i] = s[i];
			}
		}
	}
}

/**
 * Update the ACS stack according to the access operation.
 * @param o		Access operation.
 * @param a		ACS to update.
 */
void PersDomain::update(const icache::Access& o, t& a) {
	if(a.isBottom())
		return;
	update(o, a.whole());
	for(int i = 0; i < a.depth(); i++)
		update(o, a[i]);
}

/**
 * Update an ACS stack from a list of access operations.
 * @param os	List of access operations.
 * @param a		ACS to update.
 */
void PersDomain::update(const Bag<icache::Access>& os, t& a) {
	for(int i = 0; i < os.size(); i++)
		update(os[i], a);
}

/**
 * Update an ACS stack corresponding to an edge transition.
 * @param v	Source vertex.
 * @param e	Transition edge.
 * @param a	ACS stack to update.
 */
void PersDomain::update(Edge *e, t& a) {
	if(a.isBottom())
		return;

	// update source and edge
	const Bag<icache::Access>& sa = icache::ACCESSES(e->source());
	update(sa, a);
	const Bag<icache::Access>& ea = icache::ACCESSES(e);
	update(ea, a);

	// handle enter/leave from loops
	if(LOOP_EXIT_EDGE(e))
		for(LoopIter h(e->source()); h(); h++) {
			leave(a);
			if(*h == LOOP_EXIT_EDGE(e))
				break;
		}
	if(LOOP_HEADER(e->target()) && !BACK_EDGE(e)) {
		enter(a);
	}
}

/**
 * Called when a new level is entered.
 * @param a		ACS stack to update.
 */
void PersDomain::enter(t& a) {
	a.push(_top.whole());
}

/**
 * Called when a new level is left.
 * @param a		ACS stack to update.
 */
void PersDomain::leave(t& a) {
	if (a.isBottom())
		return;
	a.pop();
}

/**
 * Test if two ACS are equal.
 * @param a1	First ACS.
 * @param a2	Second ACS.
 */
bool PersDomain::equals(const ACS& a1, const ACS& a2) {
	for(int b = 0; b < n; b++)
		if(a1[b] != a2[b])
			return false;
	return true;
}

/**
 * Test if two ACS stack are equal.
 * @param a1	First ACS stack.
 * @param a2	Second ACS stack.
 */
bool PersDomain::equals(const t& a1, const t& a2) {
	if(a1.depth() != a2.depth())
		return false;
	if(a1.isBottom() != a2.isBottom())
		return false;
	if(!equals(a1.whole(), a2.whole()))
		return false;
	for(int i = 0; i < a1.depth(); i++)
		if(!equals(a1[i], a2[i]))
			return false;
	return true;
}

} }	// otawa::icat3
