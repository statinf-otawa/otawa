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
 * The first template parameter must provide a graph traversal interface. Often used implementation
 * of this interface for @ref CFG includes  @ref ForwardCFGAdapter (forward traversal) or
 * @ref BackwardCFGAdapter (backward traversal).
 *
 * @param G		Type of processed graph
 * 				(must implement concept @ref otawa::concept::DiGraphWithIndexedVertex,
 * 				@ref otawa::concept::DiGraphWithEntry and @ref otawa::concept::DiGraphWithLoop)
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

} }

#endif /* OTAWA_DFA_ABSINTLITE_H_ */
