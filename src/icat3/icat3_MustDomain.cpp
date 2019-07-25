/*
 *	icat3::MUSTDomain class interface
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
 * @class MustDomain
 * Abstract Interpretation domain implementing the MUST analysis
 * for L1 instruction cache according to:
 *
 * C. Ferdinand, F. Martin, and R. Wilhelm. Applying Compiler Techniques to Cache Behavior Prediction.
 * In Proceedings of the ACM SIGPLAN Workshop on Language, Compiler and Tool Support for Real-Time Systems,
 * pages 37--46, June 1997.
 *
 * @ingroup icat3
 */

/**
 * Build a MUST analysis domain.
 * @param coll	LBlock collection to use.
 * @param set	Current cache set.
 * @param init	Initial value (default to top).
 */
MustDomain::MustDomain(const LBlockCollection& coll, int set, const t *init)
	:	n(coll[set].count()),
		_bot(n, BOT_AGE),
		_top(n, coll.A()),
		_set(set),
		_coll(coll),
		A(coll.A()),
		_init(init ? *init : _top),
		tmp(n)
	{ }

/**
 * Perform join of given states.
 * @param d		ACS to join in.
 * @param s		ACS to join.
 */
void MustDomain::join(t& d, const t& s) {
	// J(a, a') = a" s.t.

	// ∀ b ∈ B_s, a"[b] = max(a[b], a'[b])
	for(int i = 0; i < n; i++)
		d[i] = max(d[i], s[i]);
}

/**
 * Implements the fetch cache operation.
* @param a		ACS to update.
* @param lb		LBlock to fetch.
 */
void MustDomain::fetch(t& a, const LBlock *lb) {
	int b = lb->index();

	// U(a, b) = a' s.t. ∀ b' ∈ B_s
	for(int i = 0; i < n; i++)
		// a'[b'] = a[b'] + 1	if a[b'] < a[b] ∧ a[b'] ≠ ⊥
		if(a[i] < a[b] && a[i] != BOT_AGE)
			a[i]++;
		// a'[b'] = a[b']		else

	// a[b] = 0
	a[b] = 0;
}

/**
 * Implements the update of ACS from an instruction cache access
 * operation.
 * @param o		Access operation to perform.
 * @param a		ACS to update.
 */
void MustDomain::update(const icache::Access& o, ACS& a) {
	switch(o.kind()) {

	case icache::FETCH:
		if(_coll.cache()->set(o.address()) == _set)
			fetch(a, LBLOCK(o));
		break;

	case icache::PREFETCH:
		if(_coll.cache()->set(o.address()) == _set) {
			copy(tmp, a);
			fetch(a, LBLOCK(o));
			join(a, tmp);
		}
		break;

	case icache::NONE:
		break;

	default:
		ASSERT(false);
	}
}

/**
 * Update the ACS according to a list of access operations.
 * @param os	Access operations.
 * @param a		ACS to update.
 */
void MustDomain::update(const Bag<icache::Access>& os, ACS& a) {
	for(int i = 0; i < os.size(); i++)
		update(os[i], a);
}

/**
 * Update an ACS according to an edge transition.
 * @param v		Source basic block.
 * @param e		Edge transition.
 * @param a		ACS to update.
 */
void MustDomain::update(Edge *e, ACS& a) {
	if(equals(a, bot()))
		return;
	const Bag<icache::Access>& sa = icache::ACCESSES(e->source());
	update(sa, a);
	const Bag<icache::Access>& ea = icache::ACCESSES(e);
	update(ea, a);
}

/**
 * Test if two ACS are equal.
 * @param a	First ACS.
 * @param b	Second ACS.
 */
bool MustDomain::equals(const ACS& a, const ACS& b) {
	for(int i = 0; i < n; i++)
		if(a[i] != b[i])
			return false;
	return true;
}

} }		// otawa::icat3
