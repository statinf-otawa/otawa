/*
 *	icat3::MustPersAnalysis class implementation
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

#include <elm/data/ListQueue.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/ai.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/icache/features.h>
#include <otawa/proc/EdgeProcessor.h>
#include "features.h"
#include "MustPersDomain.h"

namespace otawa { namespace icat3 {

/**
 * @class ACS
 * Represents an abstract cache state. The ACS is dedicated
 * to a particular cache state and provides for each l-block of
 * this set an age.
 */

/**
 */
ACS::ACS(void) {
}

/**
 */
ACS::ACS(int n, age_t d): AllocTable<age_t>(n) {
	fill(d);
}

/**
 */
ACS::ACS(const ACS& a): AllocTable<age_t>(a) {
}

/**
 */
ACS& ACS::operator=(const ACS& a) {
	copy(a);
	return *this;
}

/**
 * Print the current ACS.
 * @param set	Set number.
 * @param coll	Matching l-block collection.
 * @param out	Output (default cout).
 */
void ACS::print(int set, const LBlockCollection& coll, io::Output& out) const {
	out << "{ ";
	bool fst = true;
	for(int i = 0; i < coll[set].count(); i++) {
		if(fst)
			fst = false;
		else
			out << ", ";
		out << coll[set][i].address() << ": ";
		if((*this)[i] == coll.A())
			out << 'A';
		else if((*this)[i] == BOT_AGE)
			out << '_';
		else
			out << (*this)[i];
	}
	out << " }";
}

/**
 */
io::Output& operator<<(io::Output& out, const ACS& a) {
	out << "{ ";
	bool fst = true;
	for(int i = 0; i < a.count(); i++) {
		if(fst)
			fst = false;
		else
			out << ", ";
		out << i << ": ";
		if(a[i] == BOT_AGE)
			out << '_';
		else
			out << a[i];
	}
	out << " }";
	return out;
}


/**
 * @class ACSStack
 * An ACS stack is used by Persistence analysis to maintain the stack
 * of ACS relative to each level of loop nest containing the ACS.
 */

/**
 * Build bottom ACS stack.
 */
ACSStack::ACSStack(void): _bot(true) {
}

/**
 * Build an ACS stack with top value.
 * @param n		Number of l-blocks.
 */
ACSStack::ACSStack(int n): _bot(false), _whole(n) {
}

/**
 * Initialize an ACS stack.
 * @param a		ACS used to initializet the top-level.
 */
ACSStack::ACSStack(const ACS& a): _bot(false), _whole(a) {

}

/**
 * Copy the given ACS stack to the current ACS stack.
 * @param a		ACS stack to copy.
 */
void ACSStack::copy(const ACSStack& a) {
	if(a.isBottom()) {
		_bot = true;
		_stack.clear();
	}
	else {
		_bot = false;
		_whole.copy(a._whole);
		_stack.setLength(a._stack.length());
		for(int i = 0; i < _stack.length(); i++)
			_stack[i].copy(a._stack[i]);
	}
}


/**
 * Print the given ACS stack.
 * @param set	Current set.
 * @param coll	Current L-Block collection.
 * @param out	Output to use (default cout).
 */
void ACSStack::print(int set, const LBlockCollection& coll, io::Output& out) const {
	if(isBottom())
		out << "_";
	else {
		_whole.print(set, coll, out);
		for(int i = 0; i < _stack.length(); i++) {
			out << '|';
			_stack[i].print(set, coll, out);
		}
	}
}


class CFGCollectionGraph {
public:
	CFGCollectionGraph(const CFGCollection& coll): _coll(coll) { }

	typedef Block *vertex_t;
	typedef Edge *edge_t;

	inline vertex_t entry(void) const { return _coll.entry()->entry(); }
	inline vertex_t exit(void) const { return _coll.entry()->exit(); }
	inline vertex_t sinkOf(edge_t e) const { if(!e->sink()->isSynth()) return e->sink(); else return e->sink()->toSynth()->callee()->entry(); }
	inline vertex_t sourceOf(edge_t e) const { if(!e->source()->isSynth()) return e->source(); else return e->source()->toSynth()->callee()->exit(); }

	class Predecessor: public PreIterator<Predecessor, Edge *> {
	public:
		Predecessor(const CFGCollectionGraph& graph, vertex_t v) {
			if(!v->isEntry())
				i = v->ins();
			else {
				c = v->cfg()->callers();
				if(c) {
					i = c->ins();
					c++;
				}
			}
		}

		inline bool ended(void) const { return i.ended() && c.ended(); }
		inline void next(void) { i++; if(!i && c) { c++; i = c->ins(); } }
		inline Edge *item(void) const { return i; }

	private:
		Block::EdgeIter i;
		CFG::CallerIter c;
	};

	class Successor: public PreIterator<Predecessor, Edge *> {
	public:
		Successor(const CFGCollectionGraph& graph, vertex_t v) {
			if(!v->isExit())
				i = v->outs();
			else {
				c = v->cfg()->callers();
				if(c) {
					i = c->outs();
					c++;
				}
			}
		}

		inline bool ended(void) const { return i.ended() && c.ended(); }
		inline void next(void) { i++; if(!i && c) { c++; i = c->ins(); } }
		inline Edge *item(void) const { return i; }

	private:
		Block::EdgeIter i;
		CFG::CallerIter c;
	};

	class Iterator: public CFGCollection::BBIterator {
	public:
		inline Iterator(const CFGCollectionGraph& g): CFGCollection::BBIterator(&g._coll) { }
	};

	// Indexed concept
	inline int index(Block *v) const { return v->index(); }
	inline int count(void) const { return _coll.countBB(); }

private:
	const CFGCollection& _coll;
};

#define AI_DEBUG(x)		//{ x }

template <class S>
class SimpleControler {
public:
	typedef typename S::t t;
	typedef typename S::graph_t graph_t;
	typedef typename S::domain_t domain_t;
	typedef typename graph_t::vertex_t vertex_t;
	typedef typename graph_t::edge_t edge_t;

	SimpleControler(S& store)
		: d(store.domain()), g(store.graph()), s(store) { }

	void reset(void) {
		s.reset();
	}

	bool update(edge_t e) {
		vertex_t v = g.sourceOf(e);
		AI_DEBUG(cerr << "DEBUG: examining " << e << io::endl;)

		// join of predecessors
		if(v == g.entry()) {
			d.copy(tmp, d.init());
			AI_DEBUG(cerr << "DEBUG:\t\tinitial vertex!\n";)
		}
		else {
			d.copy(tmp, d.bot());
			for(typename graph_t::Predecessor pe(g, v); pe; pe++) {
				const t& v = s.get(pe);
				//AI_DEBUG(cerr << "DEBUG:     input of " << *pe << ": "; d.print(v, cerr); cerr << io::endl;)
				d.join(tmp, v);
				//AI_DEBUG(cerr << "DEBUG:     result in "; d.print(tmp, cerr); cerr << io::endl;)
			}
		}

		// update
		AI_DEBUG(cerr << "DEBUG:\t\tin  state: "; d.print(tmp, cerr); cerr << io::endl;)
		d.update(v, e, tmp);
		AI_DEBUG(cerr << "DEBUG:\t\tnew  state: "; d.print(tmp, cerr); cerr << io::endl;)
		AI_DEBUG(cerr << "DEBUG:\t\tprev state: "; d.print(s.get(e), cerr); cerr << io::endl;)

		// any change?
		bool update = !d.equals(tmp, s.get(e));
		if(update) {
			s.set(e, tmp);
			AI_DEBUG(cerr << "DEBUG:\t\t" << e << " updated!\n";)
		}
		return update;
	}

	inline graph_t graph(void) const { return g; }
	inline const typename S::domain_t& domain(void) const { return d; }

private:
	typename S::domain_t& d;
	graph_t& g;
	S& s;
	t tmp;
};

template <class C>
class BreadthFirstAI {
public:
	typedef typename C::graph_t graph_t;
	typedef typename C::domain_t domain_t;
	typedef typename graph_t::vertex_t vertex_t;

	BreadthFirstAI(C& controler): c(controler), g(c.graph()) { }

	void run(void) {
		c.reset();
		wl.put(g.entry());
		while(wl) {
			vertex_t v = wl.get();
			AI_DEBUG(cerr << "\nDEBUG: processing " << v << io::endl;)
			for(typename graph_t::Successor e(g, v); e; e++) {
				bool update = c.update(e);
				if(update) {
					vertex_t w = g.sinkOf(e);
					if(!wl.contains(w)) {
						wl.put(w);
						AI_DEBUG(cerr << "DEBUG:     putting " << w << io::endl;)
					}
				}

			}
		}
	}

private:
	C& c;
	graph_t g;
	ListQueue<vertex_t> wl;
};


/**
 */
class MustPersAnalysis: public Processor {
public:
	static p::declare reg;
	MustPersAnalysis(void): Processor(reg), coll(0), init_must(0), init_pers(0), cfgs(0) {
	}

	virtual void configure(const PropList& props) {
		Processor::configure(props);
		if(props.hasProp(MUST_INIT))
			init_must = &MUST_INIT(props);
		if(props.hasProp(PERS_INIT))
			init_pers = &PERS_INIT(props);
	}

protected:

	virtual void setup(WorkSpace *ws) {
		coll = LBLOCKS(ws);
		if(coll) {
			cfgs = otawa::INVOLVED_CFGS(ws);
			ASSERT(cfgs);
		}
	}

	virtual void processWorkSpace(WorkSpace *ws) {
		if(!coll)
			return;

		// prepare containers
		for(CFGCollection::BBIterator b(cfgs); b; b++)
			for(Block::EdgeIter e = b->outs(); e; e++) {
				(*MUST_STATE(e)).configure(*coll);
				(*PERS_STATE(e)).configure(*coll);
				track(MUST_PERS_ANALYSIS_FEATURE, MUST_STATE(e));
			}

		// compute ACS
		for(int i = 0; i < coll->cache()->setCount(); i++) {
			if((*coll)[i].count()) {
				if(logFor(LOG_FUN))
					log << "\tanalyzing set " << i << io::endl;
				processSet(i);
			}
		}
	}

	void processSet(int set) {

		// perform the computation
		MustPersDomain d(*coll, set, init_must ? &(*init_must)[set] : 0);
		CFGCollectionGraph g(*cfgs);
		typedef ai::EdgeStore<MustPersDomain, CFGCollectionGraph> store_t;
		typedef SimpleControler<store_t> controler_t;
		ai::EdgeStore<MustPersDomain, CFGCollectionGraph> s(d, g);
		controler_t c(s);
		BreadthFirstAI<controler_t> ai(c);
		ai.run();

		// store the result
		for(CFGCollection::BBIterator b(cfgs); b; b++)
			for(Block::EdgeIter e = b->outs(); e; e++) {
				(*MUST_STATE(e))[set] = d.must(s.get(e));
				(*PERS_STATE(e))[set] = d.pers(s.get(e));
			}
	}

	const LBlockCollection *coll;
	const Container<ACS> *init_must, *init_pers;
	const CFGCollection *cfgs;
};

p::declare MustPersAnalysis::reg = p::init("otawa::icat3::MustPersAnalysis", Version(1, 0, 0))
	.require(LBLOCKS_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.provide(MUST_PERS_ANALYSIS_FEATURE)
	.make<MustPersAnalysis>();


/**
 * Initial state for MUST instruction cache analysis.
 *
 * @par Feature
 * @li @ref MUST_PERS_ANALYSIS_FEATURE
 */
p::id<Container<ACS> > MUST_INIT("otawa::icat3::MUST_INIT");

/**
 * Initial state for PERS instruction cache analysis.
 *
 * @par Feature
 * @li @ref MUST_PERS_ANALYSIS_FEATURE
 */
p::id<Container<ACS> > PERS_INIT("otawa::icat3::PERS_INIT");

/**
 * Feature performing instruction cache analysis with combined
 * MUST and PERS analyzes.
 *
 * @par Configuration
 * @li @ref MUST_INIT
 * @li @ref PERS_INIT
 *
 * @par Properties
 * @li @ref MUST_STATE
 * @li @ref PERS_STATE
 */
p::feature MUST_PERS_ANALYSIS_FEATURE("otawa::icat3::MUST_PERS_ANALYSIS_FEATURE", p::make<MustPersAnalysis>());

/**
 * Properties giving the ACS for the MUST analysis at a particular
 * program point.
 *
 * @par Hooks
 * @li @ref Edge
 *
 * @par Feature
 * @li @ref MUST_PERS_ANALYSIS_FEATURE
 */
p::id<Container<ACS> > MUST_STATE("otawa::icat3::MUST_STATE");

/**
 * Properties giving the ACS for the PERS analysis at a particular
 * program point.
 *
 * @par Hooks
 * @li @ref Edge
 *
 * @par Feature
 * @li @ref MUST_PERS_ANALYSIS_FEATURE
 */
p::id<Container<ACSStack> > PERS_STATE("otawa::icat3::PERS_STATE");

} }		// otawa::icat3
