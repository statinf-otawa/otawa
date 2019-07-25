/*
 *	icat3::CatBuilder class implementation
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

#include <otawa/cfg.h>
#include <otawa/hard/Memory.h>
#include <otawa/icat3/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/program.h>

#include "MustPersDomain.h"
#include "MayDomain.h"

namespace otawa { namespace icat3 {

class CacheState {
public:
	CacheState(const LBlockCollection& coll, int set, bool may = false)
	:	must_dom(coll, set, nullptr),
		pers_dom(coll, set, nullptr),
		may_dom(may ? new MayDomain(coll, set, nullptr) : nullptr),
		must_state(nullptr),
		pers_state(nullptr),
		may_state(nullptr)
	{ }

	~CacheState(void) {
		if(may_dom != nullptr)
			delete may_dom;
		if(must_state != nullptr)
			delete must_state;
		if(pers_state != nullptr)
			delete pers_state;
		if(may_state != nullptr)
			delete may_state;

	}

	MustDomain must_dom;
	PersDomain pers_dom;
	MayDomain *may_dom;
	typename MustDomain::t *must_state;
	typename PersDomain::t *pers_state;
	typename MayDomain::t *may_state;
};

/**
 * @class ACSManager;
 * The ACS manager provides a simple way to use the results of @ref icat3
 * ACS results for the instruction cache analysis. While the analyzes are separately
 * performed set by set , this class uses the produced ACS together to
 * provide ACS information for a the access sequence of a particular block.
 *
 * The usage pattern is to call start(b) to start getting the ACS for block b
 * and to call update(a) for each access a of b in order. At each step,
 * information about MUST and PERS analysis ages can be obtained using
 * the methods mustAge() and persAge().
 *
 * @par Used features
 * @li @ref otawa::icat3::MUST_PERS_ANALYSIS_FEATURE (required)
 * @li @ref otawa::icat3::MAY_ANALYSIS_FEATURE (optional)
 *
 * @warning Processing ACS of all sets together may be expensive in memory
 * place. If you have to manage several blocks, creates only on instance
 * of ACSManager and call start() for each b (the ACSManager will automatically
 * reconfigure itself).
 *
 * @ingroup icat3
 */

/**
 * Build the ACS manager.
 * @param ws	Workspace to work on.
 */
ACSManager::ACSManager(WorkSpace *ws): coll(**icat3::LBLOCKS(ws)), _b(0) {
	_states = new CacheState *[coll.sets()];
	array::set(_states, coll.sets(), null<CacheState>());
	_has_may = ws->isProvided(icat3::MAY_ANALYSIS_FEATURE);
}

/**
 */
ACSManager::~ACSManager(void) {
	for(int i = 0; i < coll.sets(); i++)
		if(_states[i] != nullptr)
			delete _states[i];
	delete [] _states;
}

/**
 * Start using ACS information for block b.
 * @param b		Block to be explored.
 */
void ACSManager::start(Block *b) {
	ASSERT(b);
	_b = b;
	used.clear();
}

/**
 * Update the ACS considering that the access is performed.
 * @param acc	Performed access.
 */
void ACSManager::update(const icache::Access& acc) {
	LBlock *lb = LBLOCK(acc);
	CacheState& s = use(lb->set());
		use(lb->set());
	s.must_dom.update(acc, *s.must_state);
	s.pers_dom.update(acc, *s.pers_state);
	if(_has_may)
		s.may_dom->update(acc, *s.may_state);
}

/**
 * Get the age of the given l-block for MUST analysis.
 * @param lb	Looked l-block.
 * @return		Age of lb.
 */
int ACSManager::mustAge(const LBlock *lb) {
	return use(lb->set()).must_state->get(lb->index());
}

/**
 * Get the age of the given l-block for MAY analysis.
 * @param lb	Looked l-block.
 * @return		Age of lb.
 */
int ACSManager::mayAge(const LBlock *lb) {
	if(!_has_may)
		return 0;
	else
		return use(lb->set()).may_state->get(lb->index());
}

/**
 * Get the depth of the current PERS analysis ACS.
 * @param lb	Looked l-block.
 * @return		Depth of the corresponding ACS.
 */
int ACSManager::depth(const LBlock *lb) {
	return use(lb->set()).pers_state->depth();
}

/**
 * Get the age of an l-block for the PERS analysis at the given depth.
 * @param lb		Looked l-block.
 * @param depth		Looked depth.
 * @return			L-block age.
 */
int ACSManager::persAge(const LBlock *lb, int depth) {
	if(depth < 0)
		return use(lb->set()).pers_state->whole()[lb->index()];
	else
		return use(lb->set()).pers_state->get(depth)[lb->index()];
}

/**
 * Print the ACS corresponding to the given l-block.
 * @param lb	Looked l-block.
 * @param out	Output stream.
 */
void ACSManager::print(const LBlock *lb, Output& out) {
	CacheState& state = use(lb->set());
	out << "{MUST: ";
	state.must_dom.print(*state.must_state, out);
	out << ", PERS: ";
	state.pers_dom.print(*state.pers_state, out);
	if(_has_may) {
		out << ", MAY: ";
		state.may_dom->print(*state.may_state, out);
	}
	out << "}";
}

/**
 * Retrieve the cache state corresponding to the given set.
 * @param set	Considered set.
 * @return		Corresponding cache state.
 */
CacheState& ACSManager::use(int set) {
	if(!used.contains(set)) {
		if(_states[set] == nullptr)
			_states[set] = new CacheState(coll, set, _has_may);
		if(_states[set]->must_state == nullptr)
			_states[set]->must_state = new MustDomain::t;
		_states[set]->must_dom.copy(*_states[set]->must_state, (*MUST_IN(_b))[set]);
		if(_states[set]->pers_state == nullptr)
			_states[set]->pers_state = new PersDomain::t;
		_states[set]->pers_dom.copy(*_states[set]->pers_state, (*PERS_IN(_b))[set]);
		if(_has_may) {
			if(_states[set]->may_state == nullptr)
				_states[set]->may_state = new MayDomain::t;
			_states[set]->may_dom->copy(*_states[set]->may_state, (*MAY_IN(_b))[set]);
		}
	}
	return *_states[set];
}


/**
 */
class CatBuilder: public BBProcessor {
public:
	static p::declare reg;
	CatBuilder(void): BBProcessor(reg), coll(0), mem(0), A(0), man(0)  { }

protected:

	typedef MustPersDomain::t acs_t;

	/**
	 */
	virtual void setup(WorkSpace *ws) {
		coll = icat3::LBLOCKS(ws);
		ASSERT(coll);
		A = coll->A();
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);
		man = new ACSManager(ws);
	}

	/**
	 */
	virtual void cleanup(WorkSpace *ws) {
		delete man;
	}

	/**
	 */
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *v) {
		for(Block::EdgeIter e = v->outs(); e(); e++) {
			if(logFor(LOG_BLOCK))
				log << "\t\t\t\tprocess " << *e << io::endl;
			if(v->isSynth() && v->toSynth()->callee())
				man->start(v->toSynth()->callee()->exit());
			else
				man->start(v);
			processAccesses(v, *icache::ACCESSES(v));
			processAccesses(v, *icache::ACCESSES(*e));
		}
	}

	/**
	 * Generate and store the events.
	 * @param tu	Time unit to put the event in.
	 * @param place	Place of the contribution.
	 * @param cv	Counting vertex.
	 * @param iv	Input vertex.
	 * @param cont	Contribution.
	 */
	void processAccesses(Block *v, Bag<icache::Access>& accs) {
		for(int i = 0; i < accs.count(); i++) {
			LBlock *lb = LBLOCK(accs[i]);
			age_t age = man->mustAge(lb);
			category_t cat = NC;

			// in Must ACS => AH
			if(0 <= age && age < A)
				cat = AH;

			// in PERS ACS in loops?
			else if(man->depth(lb) > 0) {
				age = man->persAge(lb, man->depth(lb) - 1);
				if(0 <= age && age < A) {
					cat = PE;
					LoopIter h(v);
					for(int d = man->depth(lb) - 2; d >= 0 && h(); d--, h++) {
						age = man->persAge(lb, d);
						if(0 <= age && age < A) {
							HEADER(accs[i]) = *h;
							break;
						}
					}
				}
			}

			// in PERS at top level
			if(cat == NC) {
				age = man->persAge(lb, -1);
				if(0 <= age && age < A)
					cat = PE;
			}

			// assign category
			CATEGORY(accs[i]) = cat;
			if(logFor(LOG_BLOCK)) {
				log << "\t\t\t\t" << accs[i] << ": " << CATEGORY(accs[i]);
				if(CATEGORY(accs[i]) == PE)
					log << " (" << HEADER(accs[i]) << ")";
				log << " ACS = "; man->print(lb, log); log << io::endl;
			}

			// update the current ACS
			man->update(accs[i]);
		}
	}

	const LBlockCollection *coll;
	const hard::Memory *mem;
	int A;
	ACSManager *man;
};

p::declare CatBuilder::reg = p::init("otawa::icat3::CatBuilder", Version(1, 0, 0))
	.extend<BBProcessor>()
	.make<CatBuilder>()
	.provide(CATEGORY_FEATURE)
	.require(LBLOCKS_FEATURE)
	.require(MUST_PERS_ANALYSIS_FEATURE)
	.require(hard::MEMORY_FEATURE);


/**
 * This feature ensures that category information is stored on each @ref icache::Access found
 * in the blocks and in the CFG of the current workspace. Such information is made of
 * @ref CATEGORY specifying the behaviour of the cache (AH -- Always Hit, AM -- Always Miss,
 * PE -- Persistent and NC -- Not-classified) and, in the PE case, the loop header causing
 * the considered block to be evicted.
 *
 * @par Properties
 * @li @ref CATEGORY
 * @li @ref HEADER
 *
 * @par Default implementation
 * @li @ref CatBuilder
 *
 * @ingroup icat3
 */
p::feature CATEGORY_FEATURE("otawa::icat3::CATEGORY_FEATURE", CatBuilder::reg);


/**
 * Defines the instruction cache category for the @ref icache::Access where
 * this property is set.
 *
 * @par Hooks
 * @li @ref icache::Access
 *
 * @par Features
 * @li @ref CATEGORY_FEATURE
 *
 * @ingroup icat3
 */
p::id<category_t> CATEGORY("otawa::icat3::CATEGORY", UC);


/**
 * In the case of a category of type PE (persistent), this property
 * provides the loop header that causes the considered block to be
 * eviected.
 *
 * @par Hooks
 * @li @ref icache::Access
 *
 * @par Features
 * @li @ref CATEGORY_FEATURE
 *
 * @ingroup icat3
 */
p::id<Block *> HEADER("otawa::icat3::HEADER", 0);

} }	// otawa::icat3






