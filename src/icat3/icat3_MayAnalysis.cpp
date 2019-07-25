/*
 *	icat3::MayAnalysis class implementation
 *	Copyright (c) 2017, IRIT UPS.
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

#include <otawa/ai/ArrayStore.h>
#include <otawa/cfg/CompositeCFG.h>
#include <otawa/ai/SimpleAI.h>
#include <otawa/cfg/features.h>
#include <otawa/icache/features.h>
#include <otawa/icat3/features.h>
#include "../../include/otawa/ai/RankingAI.h"
#include "MayDomain.h"


namespace otawa { namespace icat3 {

/**
 * @class MayDomain
 * Domain for the MAY analysis for instruction cache.
 *
 * @ingroup icat3
 */

/**
 */
MayDomain::MayDomain(const LBlockCollection& coll, int set, const t *init)
:	n(coll[set].count()),
	_bot(n, BOT_AGE),
	_top(n, 0),
	_set(set),
	_coll(coll),
	A(coll.A()),
	_init(init ? *init : _top),
	tmp(n)
{ }

/*
 */
void MayDomain::join(t& d, const t& s) {
	// J(a, a') = a" s.t.

	// ∀ b ∈ B_s, a"[b] = min(a[b], a'[b])
	for(int i = 0; i < n; i++)
		if(d[i] == BOT_AGE)
			d[i] = s[i];
		else if(s[i] != BOT_AGE)
			d[i] = min(d[i], s[i]);
}

/**
 */
void MayDomain::fetch(t& a, const LBlock *lb) {
	int b = lb->index();

	// U(a, b) = a' s.t. ∀ b' ∈ B_s
	for(int i = 0; i < n; i++)
		// a'[b'] = a[b'] + 1	if a[b'] < a[b] ∧ a[b'] ≠ ⊥
		if(a[i] <= a[b] && a[i] != A)
			a[i]++;
		// a'[b'] = a[b']		else

	// a[b] = 0
	a[b] = 0;
}

/**
 */
void MayDomain::update(const icache::Access& access, t& a) {
	if(a[0] == BOT_AGE)
		return;
	switch(access.kind()) {

	case icache::FETCH:
		if(_coll.cache()->set(access.address()) == _set)
			fetch(a, LBLOCK(access));
		break;

	case icache::PREFETCH:
		if(_coll.cache()->set(access.address()) == _set) {
			copy(tmp, a);
			fetch(a, LBLOCK(access));
			join(a, tmp);
		}
		break;

	case icache::NONE:
		break;

	default:
		ASSERT(false);
	}
}


/**
 */
class MayAdapter {
public:
	typedef MayDomain domain_t;
	typedef typename domain_t::t t;
	typedef CompositeCFG graph_t;
	typedef ai::ArrayStore<domain_t, graph_t> store_t;

	MayAdapter(int set, const t *init, const LBlockCollection& coll, const CFGCollection& cfgs):
		_domain(coll, set, init),
		_graph(cfgs),
		_store(_domain, _graph) { }

	inline domain_t& domain(void) { return _domain; }
	inline graph_t& graph(void) { return _graph; }
	inline store_t& store(void) { return _store; }

	void update(const Bag<icache::Access>& accs, t& d) {
		for(auto acc = *accs; acc(); acc++)
			_domain.update(*acc, d);
	}

	void update(Block *v, t& d) {
		// ∀v ∈ V \ {ν}, IN(v) = ⊔{(w, v) ∈ E} 𝕀*(β(w, v), (v, w), IN(w))
		_domain.copy(d, _domain.bot());
		t s;

		// update and join along edges
		for(auto e = _graph.preds(v); e(); e++) {
			Block *w = e->source();
			_domain.copy(s, _store.get(w));

			// apply block
			{
				const Bag<icache::Access>& accs = icache::ACCESSES(w);
				if(accs.count() > 0)
					update(accs, s);
			}

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
 * MAY analysis for instruction cache analysis by category.
 *
 * @ingroup icat3
 */
class MayAnalysis: public Processor {
public:
	static p::declare reg;
	MayAnalysis(p::declare& r = reg): Processor(r), init_may(nullptr), coll(nullptr), cfgs(nullptr) { }

protected:

	void configure(const PropList& props) override {
		Processor::configure(props);
		if(props.hasProp(MAY_INIT))
			init_may = &MAY_INIT(props);
	}

	void setup(WorkSpace *ws) override {
		coll = LBLOCKS(ws);
		ASSERT(coll != nullptr);
		cfgs = otawa::INVOLVED_CFGS(ws);
		ASSERT(cfgs != nullptr);
	}

	void processWorkSpace(WorkSpace *ws) override {

		// prepare containers
		for(CFGCollection::BlockIter b(cfgs); b(); b++)
			(*MAY_IN(*b)).configure(*coll);

		// compute ACS
		for(int i = 0; i < coll->cache()->setCount(); i++) {
			if((*coll)[i].count()) {
				if(logFor(LOG_FUN))
					log << "\tanalyzing set " << i << io::endl;
				processSet(i);
			}
		}
	}

	void destroy(WorkSpace *ws) override {
		for(CFGCollection::BlockIter b(cfgs); b(); b++)
			MAY_IN(*b).remove();
	}

private:

	void processSet(int i) {

		// perform the analysis
		MayAdapter ada(i, init_may ? &init_may->get(i) : nullptr, *coll, *cfgs);
		ai::SimpleAI<MayAdapter> ana(ada);
		ana.run();

		// store the results
		for(CFGCollection::BlockIter b(cfgs); b(); b++)
			if(b->isBasic()) {
				ada.domain().copy((*MAY_IN(*b))[i], ada.store().get(*b));
				if(logFor(LOG_BLOCK)) {
					log << "\t\t\t" << *b << ": " << ada.domain().print(ada.store().get(*b)) << io::endl;
				}
			}

		// ranked alternative
		// TODO test it
		/*ai::RankingAI<MayAdapter, ai::PropertyRanking> ana2(ada);
		ana2.run();*/
	}

	const Container<ACS> *init_may;
	const LBlockCollection *coll;
	const CFGCollection *cfgs;
};

p::declare MayAnalysis::reg = p::init("otawa::icat3::MayAnalysis", Version(1, 0, 0))
	.require(LBLOCKS_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.provide(MAY_ANALYSIS_FEATURE)
	.make<MayAnalysis>();


/**
 * Perform the ACS analysis for the May domain, that is, computes for each cache
 * block the highest age it may have considering all execution paths.
 *
 * @par Properties
 * @li @ref MAY_IN
 *
 * @par Configuraiton
 * @li @ref MAY_INIT
 *
 * @par Implementation
 * @li @ref MayAnlysis
 *
 * @ingroup icat3
 */
p::feature MAY_ANALYSIS_FEATURE("otawa::icat3::MAY_ANALYSIS_FEATURE", p::make<MayAnalysis>());


/**
 * ACS for the MAY analysis at the entry of the corresponding block or edge.
 *
 * @par Feature
 * @li @ref MAY_ANALYSIS_FEATURE
 *
 * @par Hooks
 * @li @ref Block
 * @li @ref Edge
 *
 * @ingroup icat3
 */
p::id<Container<ACS> > MAY_IN("otawa::icat3::MAY_IN");


/**
 * Initial state for MAY instruction cache analysis.
 *
 * @par Hook
 * @li Feature configuration.
 *
 * @par Feature
 * @li @ref MAY_ANALYSIS_FEATURE
 *
 * @ingroup icat3
 */
p::id<Container<ACS> > MAY_INIT("otawa::icat3::MAY_INIT");

} };	// otawa::icat3
