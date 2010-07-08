/*
 *	$Id$
 *	AbsIntLite class interface
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

#include <elm/genstruct/VectorQueue.h>

namespace otawa { namespace dfa {

#ifdef OTAWA_AIL_DEBUG
#	define OTAWA_AILD(x)	x
#else
#	define OTAWA_AILD(x)
#endif

template <class G, class T>
class AbsIntLite {
public:

	inline AbsIntLite(const G& graph, const T& domain): g(graph), d(domain) {
		vals = new typename T::t[g.count()];
		for(int i = 0; i < g.count(); i++)
			d.set(vals[i], d.bottom());
	}

	inline void process(void) {

		// initialization
		genstruct::VectorQueue<typename G::Vertex> todo;
		d.set(vals[g.index(g.entry())], d.initial());
		for(typename G::Successor succ(g, g.entry()); succ; succ++)
			todo.put(succ);

		// loop until fixpoint
		while(todo) {
			typename G::Vertex v = todo.get();
			OTAWA_AILD(cerr << "NEXT: " << v << io::endl);

			// join of predecessor
			d.set(tmp, d.bottom());
			for(typename G::Predecessor pred(g, v); pred; pred++)
				d.join(tmp, vals[g.index(pred)]);
			OTAWA_AILD(cerr << "- JOIN IN: "; d.dump(cerr, tmp); cerr << io::endl);

			// update value
			d.update(v, tmp);
			OTAWA_AILD(cerr << "- UPDATE: "; d.dump(cerr, tmp); cerr << io::endl);

			// new value ?
			if(d.equals(d.bottom(), vals[g.index(v)]) || !d.equals(tmp, vals[g.index(v)])) {
				OTAWA_AILD(cerr << "- NEW VALUE\n");
				d.set(vals[g.index(v)], tmp);

				// add successors
				for(typename G::Successor succ(g, v); succ; succ++) {
					todo.put(*succ);
					OTAWA_AILD(cerr << "- PUTTING " << *succ << io::endl);
				}
			}
		}
	}

	inline const typename T::t& in(typename G::Vertex v) {
		d.set(tmp, d.bottom());
		for(typename G::Predecessor pred(g, v); pred; pred++)
			d.join(tmp, vals[g.index(pred)]);
		return tmp;
	}

	inline const typename T::t& out(typename G::Vertex v) const {
		return vals[g.index(v)];
	}

private:
	const G& g;
	T d;
	typename T::t *vals;
	typename T::t tmp;
};

} } // otawa::dfa

#endif /* OTAWA_DFA_ABSINTLITE_H_ */
