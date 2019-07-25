/*
 *	LoopIdentifier class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <elm/array.h>

#include "../../include/otawa/graph/LoopIdentifier.h"

namespace otawa { namespace graph {

/**
 * @class LoopIdentifier
 * Implements loop identification a digraph based on algorithm from:
 *
 * T. Wei, J. Mao, W. Zou, Y. Chen, ""A New Algorithm for Identifying Loops
 * in Decompilation", International Static Analysis Symposium, 2007.
 *
 * This algorithm identifiers loops and supports irreducible loops for which
 * a main header is identified and subsequent header are qualified as re-entry
 * headers.
 *
 * For a graph G = (V, E, 𝜈), the complexity is mostly linear in function of
 * the number of edges and of vertices, O(|V| + k × |E|), where k is the
 * unstructuredness coefficient of the graph (usually close to 1).
 *
 * @ingroup graph
 */

/*
 * NOTE: speed could be improved by using a stack instead of a recursive call.
 */

/**
 */
LoopIdentifier::LoopIdentifier(const DiGraph& graph, Vertex *entry)
:	g(graph),
	_flags(new flags_t[g.count()]),
	_DFSP(new t::uint32[g.count()]),
	_iloop(new Vertex *[g.count()]),
	_irred(false),
	_entry(entry)
{
	// initialization
	array::set(_flags, g.count(), flags_t(0));
	array::set(_DFSP, g.count(), t::uint32(0));
	array::set(_iloop, g.count(), null<Vertex>());

	// run the DFS
	DFS(_entry, 1);
}


/**
 */
Vertex *LoopIdentifier::DFS(Vertex *v, int p) {

	// prefix
	setTraversed(v);
	setDFSP(v, p);

	// process each successor
	for(Vertex::EdgeIter e = v->outs(); e(); e++) {
		Vertex *w = e->sink();

		// case A: ¬traversed(c) ⟶ next node on DFSP
		if(!isTraversed(w)) {
			Vertex *nh = DFS(w, p + 1);
			tagHead(w, nh);
		}

		// case B: traversed(w) ʌ DFSP(w) > 0 ⟶ loop on DFSP
		else if(DFSP(w) > 0) {
			setHeader(w);
			tagHead(v, w);
		}

		// case C: traversed(w) ʌ DFSP(w) = 0 ʌ iloop(w) = nil ⟶ path join
		else if(iloop(w) == null<Vertex>())
			/* do nothing */ ;

		// case D: traversed(w) ʌ DFSP(w) = 0 ʌ iloop(w) ≠ nil ʌ DFSP(iloop(w)) > 0
		// ⟶  v is in the same loop as w
		else if(DFSP(iloop(w)) > 0)
			tagHead(v, iloop(w));

		// case E: traversed(w) ʌ DFSP(w) = 0 ʌ iloop(w) ≠ nil ʌ DFSP(iloop(w)) = 0
		// ⟶ re-entry case
		else {
			Vertex *h = iloop(w);
			setReentry(w);
			setIrreducible(h);
			while(iloop(h)) {
				h = iloop(h);
				if(DFSP(h) > 0) {
					tagHead(v, h);
					break;
				}
				setIrreducible(h);
			}
		}
	}

	// suffix
	setDFSP(v, 0);
	return iloop(v);
}


/**
 * @fn bool LoopIdentifier::isHeader(Vertex *v) const;
 * Test if the given vertex is a header.
 * @param v		Tested vertex.
 * @return		True if v is a header, false else.
 */


/**
 * @fn bool LoopIdentifier::isReentry(Vertex *v) const;
 * Test if the given vertex is a re-entry.
 * @param v		Tested vertex.
 * @return		True if v is a re-enry, false else.
 */


/**
 * @fn bool LoopIdentifier::isIrreducible(void) const;
 * Test if the graph contans irreducible loops.
 * @return	True if the graph contains irreducible loops, false else.
 */


/**
 * @fn bool LoopIdentifier::isIrreducible(Vertex *v) const;
 * Test if the loop headed by v is irreducible or not.
 * @param v		Tested vertex.
 * @return		True if it is irreducible, false else.
 */


/**
 * @fn Vertex *LoopIdentifier::immediateLoop(Vertex *v) const;
 * Get the immediate loop parent of the given vertex, if any.
 * @param v	Vertex to look for immediate loop parent.
 * @return	Immediate loop parent or null (if v is not in a loop).
 */


/**
 * @fn Vertex *LoopIdentifier::loopOf(Vertex *v) const;
 * Get the header of the loop immediately containing v
 * (if v is a loop header, it is v itself).
 * @param v	Vertex to look immediate container for.
 * @return	Container loop header or null if v is not contained in a loop.
 */


/**
 */
void LoopIdentifier::tagHead(Vertex *v, Vertex *h) {

	// tagging not needed
	if(v == h || h == null<Vertex>())
		return;

	// tag back
	while(iloop(v) != null<Vertex>()) {
		Vertex *ih = iloop(v);

		// header reached: stop
		if(ih == h)
			return;

		// ih before h on DFSP: h was not detected as a header
		if(DFSP(ih) < DFSP(h)) {
			setILoop(v, h);
			v = h;
			h = ih;
		}

		// move up
		else
			v = ih;
	}
	setILoop(v, h);
}


/**
 * Test if v is contained in loop headed by h (or in a subloop).
 * @param h		Considered loop header.
 * @param v		Loop to test.
 * @return		True if v is contained in loop h, false else.
 */
bool LoopIdentifier::contains(Vertex *h, Vertex *v) const {
	if(v == h)
		return true;
	else {
		for(v = iloop(v); v; v = iloop(v))
			if(v == h)
				return true;
		return false;
	}
}


/**
 * Test if an edge is a back edge.
 * @param e	Edge to test.
 * @return	True if it is a back.
 */
bool LoopIdentifier::isBack(Edge *e) const {
	return isHeader(e->sink())
		&& contains(e->sink(), e->source());
}


/**
 * Test if an edge is an entry edge (and not a re-entry edge!).
 * @param e	Edge to test.
 * @return	True if it is an entry edge, false.
 */
bool LoopIdentifier::isEntry(Edge *e) const {
	return isHeader(e->sink())
		&& !isBack(e);
}

} }	// otawa::sgraph
