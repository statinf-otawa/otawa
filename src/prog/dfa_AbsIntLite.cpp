/*
 *	$Id$
 *	AbsIntLite class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
#ifndef OTAWA_DFA_ABSINTLITE_H_
#define OTAWA_DFA_ABSINTLITE_H_

#include <elm/io.h>

using namespace elm;

namespace otawa { namespace dfa {

#ifdef OTAWA_AIL_DEBUG
#	define OTAWA_AILD(x)	x
#else
#	define OTAWA_AILD(x)
#endif

/**
 * @class AbsIntLite
 * This class provides a light implementation for abstract interpretation.
 *
 * For details on the abstract interpretation, you may define the symbol OTAWA_AIL_DEBUG
 * and process information will be printed on standard input.
 *
 * @param G		Type of processed graph (must implement concept @ref otawa::concept::DiGraphWithIndexedVertex)
 * @param T		Type of abstract domain (must implement concept @ref otawa::concept::AbstractDomain)
 */


/**
 * @fn AbsIntLite::AbsIntLite(const G& graph, const T& domain);
 * Build the analyzer.
 * @param graph		Graph to work on.
 * @param domain	Abstract domain handler.
 */


/**
 * @fn void AbsIntLite::process(void);
 * Perform the analysis.
 */


/**
 * @fn const typename T::t& AbsIntLite::in(typename G::Vertex v);
 * Get the input domain for the given vertex.
 * @param v		Vertex to get input domain for.
 * @return		Input domain.
 */


/**
 * @fn const typename T::t& AbsIntLite::out(typename G::Vertex v) const;
 * Get the output domain for the given index.
 * @param v		Vertex to get input domain for.
 * @return		Input domain.
 */

} } // otawa::dfa


namespace otawa { namespace concept {

/**
 * Concept defining type of abstract interpretation domain.
 * This type defines the abstract domain type and defines method to handle them.
 * @param G		Type of analyzed graph.
 */
template <class G>
class AbstractDomain {
public:

	/**
	 * Type of abstract domains.
	 */
	typedef char t;

	/**
	 * Get the initial domain as input of the entry node.
	 * @return		Initial domain value.
	 */
	t initial(void) const;

	/**
	 * Get the neutral value for the join operator.
	 * @return	Neutral domain.
	 */
	t bottom(void) const;

	/**
	 * Update the given domain with the given vertex.
	 * @param v		Vertex to perform update for.
	 * @param d		Input domain before and output domain after.
	 */
	void update(typename G::Vertex v, t& d);

	/**
	 * Perform the join by computing d <- join(d, s).
	 * @param d		First operand and result domain.
	 * @param s		Second operand domain.
	 */
	void join(t& d, t s);

	/**
	 * Test if two domains are equals.
	 * @param v1	First tested domain.
	 * @param v2	Second tested domain.
	 */
	bool equals(t v1, t v2) const { return v1 == v2; }

	/**
	 * Assign a domain to another one.
	 * @param d		Destination domain.
	 * @param s		Source domain.
	 */
	void set(t& d, t s) const;

	/**
	 * Dump to the given output stream the given domain.
	 * @param out	Stream to output to.
	 * @param c		Domain to dump.
	 */
	void dump(io::Output& out, t c);
};

} } // otawa::concept

#endif /* OTAWA_DFA_ABSINTLITE_H_ */
