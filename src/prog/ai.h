/*
 *	ai module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#ifndef OTAWA_PROG_AI_H_
#define OTAWA_PROG_AI_H_

#include <elm/io.h>

namespace otawa {

class PropList;

namespace ai {

using namespace elm;

/**
 * This concept describes an @ref ai Adapter concept that may be used
 * by abstract interpretation classes like @ref otawa::ai::SimpleAI.
 * @ingroup ai
 */
template <class D, class G, class S>
class AdapterConcept {
public:

	/** Type of abstract domain. */
	typedef D domain_t;

	/** Type of graph. */
	typedef G graph_t;

	/** Type of abstract domain storage. */
	typedef S store_t;

	/**
	 * Called to update the abstract state passed in d.
	 * @param v		Vertex to update with.
	 * @param d		Input and output abstract state.
	 */
	void update(typename G::vertex_t v, typename D::t& d);

	/**
	 * Get the current domain object.
	 * @return	Current domain.
	 */
	domain_t& domain(void) const;

	/**
	 * Get the current graph objecy.
	 * @return	Current graph.
	 */
	graph_t& graph(void) const;

	/**
	 * Get the current abstract state storage.
	 * @return	Abstract state storage.
	 */
	store_t& store(void) const;
};

/**
 * Concept class that must be implemented by abstract domain
 * to perform abstract interpretation.
 *
 * The value must be a Complete Partial Order (T, <=).
 *
 * @param T		Type of the domain values.
 * @param A		Type of interpreted actions.
 *
 * @ingroup ai
 */
template <class T, class A>
class DomainConcept {
public:

	/** Type of the values of the domain. */
	typedef T t;

	/** Get the initial value (at graph entry). */
	t init(void);

	/** Get the smallest value of the domain "_"
	 *  such that forall d in domain, join(_, d) = joind(d, _) = d */
	t bot(void);

	/**
	 * Perform the join of two values, d = join(d1, d2)
	 * and must satisfy the property: d1 <= d and d2 <= d.
	 * @param d		First value to join and recipient of the result.
	 * @param s		Second value to join.
	 */
	void join(t& d, t s);

	/**
	 *  Test if two abstract values are equal.
	 * 	@param v1	First value.
	 *  @param v2	Secondd value.
	 *	@return		True if both values are equal. */
	bool equals(t v1, t v2);

	/**
	 *  Copy the value s to the value d.
	 * 	@param d	Assigned value.
	 * 	@param s	Value to assign.
	 */
	void copy(t& d, t s);

	/**
	 * Only required if you use debug capabilities. It allows to
	 * display value.
	 * @param out	Stream to output to.
	 * @param c		Value to output.
	 */
	void print(io::Output& out, const t& c);

	/**
	 * Update the value according to an interpreted action.
	 * Notice that this function must be monotonic:
	 * forall domain values v1, v2 such that v1 <= v2, update(v1) <= update(v2).
	 * @param a		Current vertex.
	 * @param d		Input value and result value.
	 */
	void update(const A& a, t& d);

};

/**
 * Concept to represent a ranking function.
 */
class RankingConcept {
public:

	/**
	 * Obtain the rank given in the property list by the identifier @ref RANK_OF.
	 * @param props		Property list to look in.
	 */
	int rankOf(const PropList& props);

	/**
	 * Obtain the rank given in the property list by the identifier @ref RANK_OF.
	 * @param props		Property list to look in.
	 */
	int rankOf(const PropList *props);
};

} }	// otawa::ai

#endif /* OTAWA_PROG_AI_H_ */
