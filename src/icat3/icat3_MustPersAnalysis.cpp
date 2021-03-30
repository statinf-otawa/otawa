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

#include <elm/avl/Map.h>
#include <elm/data/ListQueue.h>
#include <otawa/ai/CFGCollectionGraph.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/ai.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/icache/features.h>
#include <otawa/proc/EdgeProcessor.h>
#include <otawa/icat3/features.h>
#include <otawa/cfg/CompositeCFG.h>
#include <otawa/ai/SimpleAI.h>
#include "MustPersDomain.h"

#define DEBUG(x)	// cerr << "DEBUG: " << x << io::endl

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


/*
 * The set of CFG making a task is viewed with this class a tuple T = (V, E, F, ν, φ) where:
 * V is the set of all blocks of all CFG including basic blocks, ν_f or ω_f entry and exit point of function f,
 * E ⊆ V × Vs the set of edges,
 * F ⊆ V × V contains the pair of (entry, exit) blocks of the functions in the task,
 * ν is the entry of the entry function of the task — this means ∃ω ∈ V ∧ (ν, ω) ∈ F,
 * φ: V → F ∪ { ⊥ }  associates to a function call the pair of entry, exit blocks of the function or ⊥ if the block is not a call.
 */

#define AI_DEBUG(x)		//{ x }


/**
 */
class MustPersAdapter {
public:
	typedef MustPersDomain domain_t;
	typedef typename domain_t::t t;
	typedef CompositeCFG graph_t;
	typedef ai::ArrayStore<domain_t, graph_t> store_t;

	MustPersAdapter(int set, const MustDomain::t *must_init, const ACS *pers_init, const LBlockCollection& coll, const CFGCollection& cfgs):
		_domain(coll, set, must_init, pers_init),
		_graph(cfgs),
		_store(_domain, _graph) { }

	inline domain_t& domain(void) { return _domain; }
	inline graph_t& graph(void) { return _graph; }
	inline store_t& store(void) { return _store; }

	void update(const Bag<icache::Access>& accs, t& d) {
		for(auto acc = *accs; acc(); acc++) {
			DEBUG("    " << *acc);
			_domain.update(*acc, d);
			DEBUG("    " << _domain.print(d));
		}
	}

	void update(Block *v, t& d) {
		// ∀v ∈ V \ {ν}, IN(v) = ⊔{(w, v) ∈ E} 𝕀*(β(w, v), (v, w), IN(w))
		_domain.copy(d, _domain.bot());
		t s;

		// update and join along edges
		DEBUG("compute IN of " << v);
		for(auto e = _graph.preds(v); e(); e++) {
			Block *w = e->source();
			_domain.copy(s, _store.get(w));
			DEBUG("  by " << *e << " in " << v->cfg());
			DEBUG("    " << _domain.print(s));

			// apply block
			{
				const Bag<icache::Access>& accs = icache::ACCESSES(w);
				if(accs.count() > 0)
					update(accs, s);
			}

			// loop support
			if(LOOP_EXIT(*e))
				for(LoopIter h(e->source()); h(); h++) {
					_domain.leaveLoop(s);
					if(*h == LOOP_EXIT(*e))
						break;
				}
			if(LOOP_HEADER(e->target()) && !BACK_EDGE(*e))
				_domain.enterLoop(s);

			// subprogram call support (PERS ACS level fix)
			if(_graph.isReturn(*e))
				_domain.doReturn(s, v);
			if(_graph.isCall(*e))
				_domain.doCall(s, e->sink()->outs()->sink());

			// apply edge
			{
				const Bag<icache::Access>& accs = icache::ACCESSES(*e);
				if(accs.count() > 0)
					update(accs, s);
			}

			// merge result
			_domain.join(d, s);
		}
	}

private:
	domain_t _domain;
	graph_t _graph;
	store_t _store;
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
		for(CFGCollection::BlockIter b(cfgs); b(); b++) {
			(*MUST_IN(*b)).configure(*coll);
			(*PERS_IN(*b)).configure(*coll);
			track(MUST_PERS_ANALYSIS_FEATURE, MUST_IN(*b));
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

		// perform the analysis
		MustPersAdapter ada(set, init_must ? &init_must->get(set) : nullptr, init_pers ? &init_pers->get(set) : nullptr, *coll, *cfgs);
		ai::SimpleAI<MustPersAdapter> ana(ada);
		ana.run();

		// store the results
		for(CFGCollection::BlockIter b(cfgs); b(); b++)
			if(b->isBasic()) {
				ada.domain().mustDomain().copy((*MUST_IN(*b))[set], ada.store().get(*b).must);
				ada.domain().persDomain().copy((*PERS_IN(*b))[set], ada.store().get(*b).pers);
				if(logFor(LOG_BLOCK)) {
					log << "\t\t\t" << *b << ": " << ada.domain().print(ada.store().get(*b)) << io::endl;
				}
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
