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
#include <otawa/ai/CFGCollectionGraph.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/ai.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/icache/features.h>
#include <otawa/proc/EdgeProcessor.h>
#include <otawa/icat3/features.h>
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
ACS::ACS(int n, age_t d): AllocArray<age_t>(n) {
	fill(d);
}

/**
 */
ACS::ACS(const ACS& a): AllocArray<age_t>(a) {
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
		out << coll[set][i]->address() << ": ";
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
 * @param d		Default age.
 */
ACSStack::ACSStack(int n, age_t d): _bot(false), _whole(n, d) {
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


/**
 * The set of CFG making a task is viewed with this class a tuple T = (V, E, F, ν, φ) where:
 * V is the set of all blocks of all CFG including basic blocks, ν_f or ω_f entry and exit point of function f,
 * E ⊆ V × Vs the set of edges,
 * F ⊆ V × V contains the pair of (entry, exit) blocks of the functions in the task,
 * ν is the entry of the entry function of the task — this means ∃ω ∈ V ∧ (ν, ω) ∈ F,
 * φ: V → F ∪ { ⊥ }  associates to a function call the pair of entry, exit blocks of the function or ⊥ if the block is not a call.
 */

#define AI_DEBUG(x)		//{ x }

template <class S>
class DefaultEdgeControler {
public:
	typedef typename S::t t;
	typedef typename S::graph_t graph_t;
	typedef typename S::domain_t domain_t;
	typedef typename graph_t::vertex_t vertex_t;
	typedef typename graph_t::edge_t edge_t;

	DefaultEdgeControler(S& store)
		: d(store.domain()), g(store.graph()), s(store) { }

	void reset(void) {
		s.reset();
	}

	bool update(vertex_t v) {
		AI_DEBUG(cerr << "DEBUG: examining " << v << io::endl;)

		// entry of a CFG
		if(v == g.entry()) {
			s.set(v, d.init());
			AI_DEBUG(cerr << "DEBUG:     INIT = "; d.print(d.init(), cerr); cerr << io::endl;)
			return true;
		}

		// ⊔{(w, v) ∈ E} U(w, v, IN[w])
		d.copy(t1, d.bot());
		if(g.isEntry(v))
			for(typename graph_t::Callers c = g.callers(v); c; c++)
				for(typename graph_t::Predecessor e = g.preds(c); e; e++)
					doCall(g.sourceOf(e), c);
		else
			for(typename graph_t::Predecessor e = g.preds(v); e; e++)
				if(g.isCall(g.sourceOf(e)))
					doReturn(g.exitOf(g.sourceOf(e)), v);
				else
					join(e);

		// any update?
		AI_DEBUG(cerr << "DEBUG:\t"; d.print(t1, cerr); cerr << " = "; d.print(s.get(v), cerr); cerr << io::endl;)
		if(d.equals(t1, s.get(v)))
			return false;
		else {
			AI_DEBUG(cerr << "DEBUG:\t\t" << v << " updated!\n";)
			s.set(v, t1);
			return true;
		}
	}

	inline graph_t& graph(void) const { return g; }
	inline const typename S::domain_t& domain(void) const { return d; }

private:

	void doCall(typename graph_t::vertex_t v, typename graph_t::vertex_t w) {
		d.copy(t2, s.get(v));
		AI_DEBUG(cerr << "DEBUG:     entering IN(" << v << ") = "; d.print(t2, cerr); cerr << io::endl;)
		d.doCall(t2, w);
		AI_DEBUG(cerr << "DEBUG:     resulting in "; d.print(t2, cerr); cerr << io::endl;)
		d.join(t1, t2);
		AI_DEBUG(cerr << "DEBUG:     joined to "; d.print(t1, cerr); cerr << io::endl;)
	}

	void doReturn(typename graph_t::vertex_t v, typename graph_t::vertex_t w) {
		d.copy(t2, s.get(v));
		AI_DEBUG(cerr << "DEBUG:     leaving IN(" << v << ") = "; d.print(t2, cerr); cerr << io::endl;)
		d.doReturn(t2, w);
		AI_DEBUG(cerr << "DEBUG:     resulting in "; d.print(t2, cerr); cerr << io::endl;)
		d.join(t1, t2);
		AI_DEBUG(cerr << "DEBUG:     joined to "; d.print(t1, cerr); cerr << io::endl;)
	}

	void join(typename graph_t::vertex_t v) {
		AI_DEBUG(cerr << "DEBUG:     joining IN(" << v << ") = "; d.print(s.get(v), cerr); cerr << io::endl;)
		d.join(t1, s.get(v));
		AI_DEBUG(cerr << "DEBUG:     result in "; d.print(t1, cerr); cerr << io::endl;)
	}

	void join(typename graph_t::edge_t e) {
		vertex_t w = g.sourceOf(e);
		d.copy(t2, s.get(w));
		AI_DEBUG(cerr << "DEBUG:     IN(" << e << ") = "; d.print(t2, cerr); cerr << io::endl;)
		d.update(e, t2); // update with the ACCESSes on both the predecessor w and e
		AI_DEBUG(cerr << "DEBUG:     updated by " << e << ": "; d.print(t2, cerr); cerr << io::endl;)
		d.join(t1, t2);
		AI_DEBUG(cerr << "DEBUG:     result in "; d.print(t1, cerr); cerr << io::endl;)
	}

	typename S::domain_t& d;
	graph_t& g;
	S& s;
	t t1, t2;
};

template <class C>
class BreadthFirstDriver {
public:
	typedef typename C::graph_t graph_t;
	typedef typename C::domain_t domain_t;
	typedef typename graph_t::vertex_t vertex_t;

	BreadthFirstDriver(C& controler): c(controler), g(c.graph()) { }

	void run(void) {
		c.reset();
		// TODO --> avoid an if in controller
		//for(typename graph_t::Successor e = g.succs(g.entry()); e; e++)
		//	wl.put(g.sinkOf(e));
		wl.put(g.entry());
		while(wl) {
			vertex_t v = wl.get();
			AI_DEBUG(cerr << "DEBUG: processing " << v << io::endl;)
			bool update = c.update(v);
			if(update) {
				if(g.isCall(v))
					put(g.entryOf(v));
				else if(g.isExit(v))
					for(typename graph_t::Callers c = g.callers(v); c; c++)
						for(typename graph_t::Successor e = g.succs(c); e; e++)
							put(g.sinkOf(e));
				else
					for(typename graph_t::Successor e = g.succs(v); e; e++)
						put(g.sinkOf(e));
			}
			AI_DEBUG(cerr << io::endl;)
		}
	}

private:

	void put(vertex_t v) {
		if(!wl.contains(v)) {
			wl.put(v);
			AI_DEBUG(cerr << "DEBUG:     putting " << v << io::endl;)
		}
	}

	C& c;
	graph_t& g;
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
		for(CFGCollection::BBIterator b(cfgs); b; b++) {
			(*MUST_IN(b)).configure(*coll);
			(*PERS_IN(b)).configure(*coll);
			track(MUST_PERS_ANALYSIS_FEATURE, MUST_IN(b));
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
		ai::CFGCollectionGraph g(*cfgs);
		typedef ai::ArrayStore<MustPersDomain, ai::CFGCollectionGraph> store_t;
		typedef DefaultEdgeControler<store_t> controler_t;
		store_t s(d, g);
		controler_t c(s);
		BreadthFirstDriver<controler_t> ai(c);
		ai.run();

		// store the result
		for(CFGCollection::BBIterator b(cfgs); b; b++) {
			(*MUST_IN(b))[set] = d.must(s.get(b));
			(*PERS_IN(b))[set] = d.pers(s.get(b));
		}
	}

	const LBlockCollection *coll;
	const Container<ACS> *init_must, *init_pers;
	const CFGCollection *cfgs;
};

p::declare MustPersAnalysis::reg = p::init("otawa::icat3::MustPersAnalysis", Version(1, 0, 0))
	.require(LBLOCKS_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
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
p::id<Container<ACS> > MUST_IN("otawa::icat3::MUST_IN");

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
p::id<Container<ACSStack> > PERS_IN("otawa::icat3::PERS_IN");

} }		// otawa::icat3
