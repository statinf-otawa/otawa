/*
 *	dfa::Lexicon class
 *	Copyright (c) 2019, IRIT UPS.
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
#ifndef OTAWA_DFA_LEXICON_H_
#define OTAWA_DFA_LEXICON_H_

#include <elm/data/custom.h>
#include <elm/data/HashMap.h>
#include <elm/data/List.h>
#include <elm/sys/Path.h>

namespace otawa { namespace dfa {

using namespace elm;

template <class S, class A, class H = HashKey<S>, class AA = DefaultAlloc >
class Lexicon {

	class DerefHash: public H {
	public:
		inline t::hash computeHash(const S *s) const { return H::computeHash(*s); }
		inline int isEqual(const S *s1, const S *s2) const { return H::isEqual(*s1, *s2); }
	};

public:

	class Handle {
		friend class Lexicon;
	public:
		inline const S& state() const { return s; }
		inline const S& operator*() const { return s; }
#		ifdef OTAWA_LEXICON_STAT
			inline void setAlive() { alive = true; }
#		endif

	private:
		inline Handle(const S& state): s(state) {
#			ifdef OTAWA_LEXICON_STAT
				alive = false;
#			endif
		}
		S s;
		List<Pair<A, Handle *> > us;
		List<Pair<Handle *, Handle *> > js;
#		ifdef OTAWA_LEXICON_STAT
			bool alive;
#		endif
	};

	static const int default_size = 4093;
	Lexicon(int size = default_size): h(size) { }
	virtual ~Lexicon() { for(auto n: h) h.allocator().free(n); }

	Handle *add(const S& s) {
		if(h.hasKey(&s))
			return h[&s];
		Handle *ha = new /*(h.manager().allocate<Handle>())*/ Handle(s);
		h[&s] = ha; return ha;
	}

	Handle *update(Handle *ha, const A& a) {
		for(auto t: ha->us)
			if(t.fst == a)
				return t.snd;
		S r;
		doUpdate(ha->s, a, r);
		Handle *rh = add(r);
		ha->us.add(pair(a, rh));
		return rh;
	}

	Handle *join(Handle *h1, Handle *h2) {
		if(h1 > h2)
			swap(h1, h2);
		for(auto j: h1->js)
			if(j.fst == h1)
				return j.snd;
		S r;
		doJoin(h1->s, h2->s, r);
		Handle *rh = add(r);
		h1->js.add(pair(h2, rh));
		return rh;
	}

#	ifdef OTAWA_LEXICON_STAT
	inline int stateCount() const { return h.count(); }

	inline int minUpdateTrans() const {
		int m = type_info<int>::max;
		for(auto ha: h)
			m = min(m, ha->us.count());
		return m;
	}

	inline int maxUpdateTrans() const {
		int m = 0;
		for(auto ha: h)
			m = max(m, ha->us.count());
		return m;
	}

	inline int countAlive() const {
		int c = 0;
		for(auto ha: h)
			if(ha->alive)
				c++;
		return c;
	}
#	endif

#	ifdef OTAWA_LEXICON_DUMP
	void dump(sys::Path p) {
		int c = 0;
		io::OutStream *outs = p.write();
		io::Output out(*outs);
		out << "digraph updates {\n";
		for(auto ha: h)
			out << '"' << (void *)ha << '"'
				 << " [label=\"" << **ha << "\"];\n";
		for(auto ha: h) {
			for(auto u: ha->us)
				out << '"' << (void *)ha << "\" -> "
					 << '"' << (void *)u.snd << '"'
					 << " [label=\"U(" << u.fst << ")\"];\n";
			for(auto j: ha->js) {
				out << c << " [shape=diamond, label=\"\"];\n";
				out << '"' << (void *)ha << "\" -> " << c << ";\n"
					 << '"' << (void *)j.fst << "\" -> " << c << ";\n"
					 << c << " -> \"" << j.snd << "\"\n;\n";
				c++;
			}
		}
		out << "}\n";
		out.flush();
		delete outs;
	}
#	endif

protected:
	virtual void doUpdate(const S& s, const A& a, S& r) = 0;
	virtual void doJoin(const S& s1, const S& s2, S& r) = 0;

private:
	HashMap<const S *, Handle *, DerefHash, AA> h;
};

} }	// otawa::dfa

#endif /* OTAWA_DFA_LEXICON_H_ */
