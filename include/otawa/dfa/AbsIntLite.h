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
#include <elm/util/BitVector.h>

namespace otawa { namespace dfa {

#ifdef OTAWA_AIL_DEBUG
#	define OTAWA_AILD(x)	x
#else
#	define OTAWA_AILD(x)
#endif

// AbsIntLiteEx class
template <class G, class T>
class AbsIntLiteEx {
public:

	inline AbsIntLiteEx(const G& graph, T& domain): g(graph), d(domain), set(g.count()) {
		vals = new typename T::t[g.count()];
	}

	inline void push(typename G::Vertex v) {
		if(!set.bit(g.index(v))) {
			set.set(g.index(v));
			todo.put(v);
		}
	}
	
	inline typename G::Vertex pop(void) {
		typename G::Vertex v = todo.get();
		set.clear(g.index(v));
		return v;
	}

	inline void process(void) {

		// set all values to bottom
		for(int i = 0; i < g.count(); i++)
			d.set(vals[i], d.bottom());

		// initialization
		d.set(vals[g.index(g.entry())], d.initial());
		OTAWA_AILD(cerr << "INITIAL: " << g.entry() << ": "; d.dump(cerr, vals[g.index(g.entry())]); cerr << io::endl);

		// first step
		/*for(typename G::Iterator v(g); v; v++)
			if(v != g.entry())
				step(v);*/
		for(typename G::Successor succ(g, g.entry()); succ; succ++)
			push(g.sinkOf(*succ));

		// loop until fixpoint
		OTAWA_AILD(cerr << "\nINTERPRETING\n");
		while(todo) {
			typename G::Vertex v = pop();
			OTAWA_AILD(cerr << "NEXT: " << v << io::endl);
			step(v);
		}
	}
	
	inline void step(typename G::Vertex v) {
		OTAWA_AILD(cerr << "processing " << v << io::endl);

		// join of predecessor
		in(v);
		OTAWA_AILD(cerr << "	- JOIN IN: "; d.dump(cerr, tmp); cerr << io::endl);

		// update value
		d.update(v, tmp);
		OTAWA_AILD(cerr << "	- UPDATE: "; d.dump(cerr, tmp); cerr << io::endl);

		// new value ?
		if(!d.equals(tmp, vals[g.index(v)])) {
			OTAWA_AILD(cerr << "	- NEW VALUE\n");
			d.set(vals[g.index(v)], tmp);

			// add successors
			for(typename G::Successor succ(g, v); succ; succ++) {
				push(g.sinkOf(*succ));
				OTAWA_AILD(cerr << "	- PUTTING " << *succ << io::endl);
			}
		}
	}

	inline const typename T::t& in(typename G::Vertex v) {
		d.set(tmp, d.bottom());
		for(typename G::Predecessor pred(g, v); pred; pred++) {
			d.set(t2, vals[g.index(g.sourceOf(*pred))]);
			OTAWA_AILD(cerr << " - FILTER " << g.sourceOf(*pred) << "("; d.dump(cerr, t2); cerr << ") = ");
			d.filter(*pred, t2);
			OTAWA_AILD(d.dump(cerr, t2); cerr << io::endl);
			d.join(tmp, t2);
			OTAWA_AILD(cerr << " - JOIN for in = "; d.dump(cerr, tmp); cerr << io::endl);
		}
		if(g.isLoopHeader(v))
			d.widen(v, tmp);
		return tmp;
	}

	inline const typename T::t& out(typename G::Vertex v) const {
		return vals[g.index(v)];
	}

	class DomainIter: public PreIterator<DomainIter, typename T::t &> {
	public:
		inline DomainIter(const AbsIntLiteEx& _ail): ail(_ail), i(-1) { }
		inline DomainIter(const DomainIter& iter): ail(iter.ail), i(iter.i) { }
		inline bool ended(void) const { return i >= ail.g.count(); }
		inline void next(void) { i++; }
		inline typename T::t& item(void) { if(i == -1) return tmp; else return vals[i]; }
	private:
		const AbsIntLiteEx& ail;
		int i;
	};

private:
	const G& g;
	T& d;
	typename T::t *vals;
	typename T::t tmp, t2;
	genstruct::VectorQueue<typename G::Vertex> todo;
	BitVector set;
};


// AbsIntLiteExtender class
template <class G, class D>
class AbsIntLiteExtender {
public:
	typedef typename D::t t;
	inline AbsIntLiteExtender(D& domain): dom(domain) { }
	inline t initial(void) { return dom.initial(); }
	inline t bottom(void) { return dom.bottom();  }
	inline void join(t& d, const t& s) { dom.join(d, s); }
	inline bool equals(const t& v1, const t& v2) { return dom.equals(v1, v2); }
	inline void set(t& d, const t& s) { dom.set(d, s); }
	inline void dump(io::Output& out, const t& v) { dom.dump(out, v); }
	inline void update(typename G::Vertex v, t &d) { dom.update(v, d); }
	inline void filter(typename G::Edge e, t &d) { }
	inline void widen(typename G::Vertex v, t &d) { }
private:
	D& dom;
};

// AbsIntLite class
template <class G, class T>
class AbsIntLite: public AbsIntLiteEx<G, AbsIntLiteExtender<G, T> > {
public:
	inline AbsIntLite(const G& graph, T& domain): AbsIntLiteEx<G, AbsIntLiteExtender<G, T> >(graph, e), e(domain) { }
private:
	AbsIntLiteExtender<G, T> e;
};

} } // otawa::dfa

#endif /* OTAWA_DFA_ABSINTLITE_H_ */
