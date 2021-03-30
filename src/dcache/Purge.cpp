/*
 *	otawa::dcache::PurgeAnalysis class
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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

#include <otawa/dcache/features.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/dcache/MUSTPERS.h>
#include <otawa/dcache/ACSMayBuilder.h>
#include <otawa/hard/Memory.h>

using namespace otawa::cache;

namespace otawa { namespace dcache {

io::Output& operator<<(io::Output& out, purge_t purge) {
	switch(purge) {
	case INV_PURGE:		out << "inv"; break;
	case NO_PURGE:		out << "no"; break;
	case PERS_PURGE:	out << "pers"; break;
	case MAY_PURGE:		out << "may"; break;
	case MUST_PURGE:	out << "must"; break;
	}
	return out;
}

/**
 * Compute purge categories based on dirtyness analysis for write-back caches.
 *
 * @ingroup dcache
 */
class PurgeAnalysis: public Processor {
public:
	static p::declare reg;
	PurgeAnalysis(p::declare& r = reg): Processor(r), cache(nullptr), mem(nullptr) { }

protected:

	virtual void processWorkSpace(WorkSpace *ws) {

		// get the cache
		const hard::CacheConfiguration *conf = hard::CACHE_CONFIGURATION_FEATURE.get(ws);
		ASSERT(conf);
		cache = conf->dataCache();
		ASSERT(cache);

		// get the memory
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);

		// get the block collection
		const BlockCollection *colls = DATA_BLOCK_COLLECTION(ws);
		ASSERT(colls);

		// process the sets
		for(int set = 0; set < cache->setCount(); set++)
			if(colls[set].count())
				processSet(ws, colls[set]);
	}

private:

	class State {
	public:
		State(WorkSpace *ws, const hard::Cache *cache, const BlockCollection& _coll)
		:	coll(_coll),
		 	dman(_coll),
			must_prob(coll.count(), coll.cacheSet(), ws, cache, cache->wayCount()),
			must_dom(coll.count(), cache->wayCount()),
			may_prob(_coll, ws, cache),
			may_dom(coll.count(), cache->wayCount()) { }

		void start(BasicBlock *bb) {
			const AllocArray<DirtyManager::t>& dstates = DIRTY(bb);
			dstate = dstates[coll.cacheSet()];
			acs_table_t *musts = MUST_ACS(bb);
			must_dom = *(musts->get(coll.cacheSet()));
			acs_table_t *mays = MAY_ACS(bb);
			may_dom = *(mays->get(coll.cacheSet()));
		}

		void update(const BlockAccess& access) {
			dman.update(dstate, access);
			must_prob.update(must_dom, access);
			may_prob.update(may_dom, access);
		}

		const BlockCollection& coll;
		DirtyManager dman;
		DirtyManager::t dstate;
		MUSTProblem must_prob;
		MUSTProblem::Domain must_dom;
		MAYProblem may_prob;
		MAYProblem::Domain may_dom;
	};

	/**
	 * Perform analysis for the given set.
	 * @param ws	Current workspace.
	 * @param coll	Current block collection.
	 */
	void processSet(WorkSpace *ws, const BlockCollection& coll) {
		if(logFor(LOG_FILE))
			log << "\tset " << coll.cacheSet() << io::endl;
		const CFGCollection *cfgs = otawa::INVOLVED_CFGS(ws);
		State state(ws, cache, coll);
		for(int i = 0; i < cfgs->count(); i++)
			for(CFG::BlockIter bb = cfgs->get(i)->blocks(); bb(); bb++)
					processBB(coll.cacheSet(), *bb, state);
	}

	/**
	 * Perform the analysis for the current set and BB.
	 * @param set	Current set.
	 * @param b		Current BB.
	 * @param state	Group together dirty, must, pers and may states.
	 */
	void processBB(int set, otawa::Block *b, State& state) {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();
		state.start(bb);
		if(logFor(LOG_BLOCK))
			log << "\t\t" << bb << io::endl;
		auto& accesses = *DATA_BLOCKS(bb);
		for(int i = 0; i < accesses.count(); i++) {
			processAccess(accesses[i], set, state);
			state.update(accesses[i]);
		}
	}


	/**
	 * Perform the analysis for the given set and block access.
	 * @param access	Access to analyze.
	 * @param set		Current set.
	 * @param state		Dirty, must, pers and may states.
	 */
	void processAccess(BlockAccess& access, int set, State& state) {

		// is the block in the current set ?
		switch(access.kind()) {
		case BlockAccess::ANY:
			break;
		case BlockAccess::BLOCK:
			if(access.block().set() != set)
				return;
			break;
		case BlockAccess::RANGE:
			if(!access.inSet(set, cache))
				return;
			break;
		}

		// log
		if(logFor(LOG_INST))
			log << "\t\t\t" << access << " as ";

		// look for the category
		ot::time time = 0;
		purge_t purge = INV_PURGE;
		switch(dcache::CATEGORY(access)) {
		case ALWAYS_HIT:
			purge = NO_PURGE;
			break;
		case FIRST_MISS:
			if(mayPurge(access, state, time))
				purge = PERS_PURGE;
			else
				purge = NO_PURGE;
			break;
		case ALWAYS_MISS:
			if(!mayPurge(access, state, time))
				purge = NO_PURGE;
			else if(!mustPurge(access, state, time))
				purge = MAY_PURGE;
			else
				purge = MUST_PURGE;
			break;
		case NOT_CLASSIFIED:
			if(mayPurge(access, state, time))
				purge = MAY_PURGE;
			else
				purge = NO_PURGE;
			break;
		case INVALID_CATEGORY:
		case FIRST_HIT:
		case TOP_CATEGORY:
			ASSERTP(false, "unsupported category");
			break;
		}

		// apply it
		if(purge >= PURGE(access)) {
			PURGE(access) = purge;
			PURGE_TIME(access) = time;
			if(logFor(LOG_INST))
				cerr << purge << io::endl;
		}
	}

	/**
	 * Look if a dirty block must be purged.
	 * @param access	Current access.
	 * @param state		Current state.
	 * @param time		Purge time.
	 * @return			True if a block must purged, false else.
	 */
	bool mustPurge(const BlockAccess& access, State& state, ot::time& time) {
		bool purged = false;
		for(int i = 0; i < state.coll.cacheSet(); i++)
			if(state.must_dom.getAge(i) == cache->blockCount() - 1
			&& state.dstate.mustBeDirty(i)
			&& (access.isAny()|| !access.inRange(state.coll[i].index()))) {
				purged = true;
				time = max(time, mem->writeTime(state.coll[i].address()));
				if(time == mem->worstWriteTime())
					break;
			}
		return purged;
	}

	/**
	 * Look if a dirty block may be purged.
	 * @param access	Current access.
	 * @param state		Current state.
	 * @param time		Purge time
	 * @return			True if a block must purged, false else.
	 */
	bool mayPurge(const BlockAccess& access, State& state, ot::time& time) {
		bool purged = false;
		if(state.may_dom.containsAny()) {
			purged = true;
			time = mem->worstWriteTime();
		}
		else
			for(int i = 0; i < state.coll.count(); i++)
				if((access.isAny() || !access.in(state.coll[i]))
				&& state.may_dom.getAge(i) == cache->blockCount() - 1
				&& state.dstate.mayBeDirty(i)) {
					purged = true;
					time = max(time, mem->writeTime(state.coll[i].address()));
					if(time == mem->worstWriteTime())
						break;
				}
		return purged;
	}

	const hard::Cache *cache;
	const hard::Memory *mem;
};

p::declare PurgeAnalysis::reg = p::init("otawa::dcache::PurgeAnalysis", Version(1, 0, 0))
	.base(CFGProcessor::reg)
	.maker<PurgeAnalysis>()
	.require(MUST_ACS_FEATURE)
	.require(MAY_ACS_FEATURE)
	.require(DATA_BLOCK_FEATURE)
	.require(DIRTY_FEATURE)
	.require(CATEGORY_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.provide(PURGE_FEATURE);


/**
 * The purge features use the dirtyness analysis for a write-back cache to categorize cache
 * miss in order to know if a write back to memory of the cache block content is needed.
 * The categories are must purge, may purge and persistant purge (only on first access in a loop).
 *
 * @par Processors
 *	* PurgeAnalysis (default)
 *
 * @par Properties
 *	* PURGE
 *	* PURGE_TIME
 *
 * @ingroup dcache
 */
p::feature PURGE_FEATURE("otawa::dcache::PURGE_FEATURE", new Maker<PurgeAnalysis>());


/**
 * This property is set to data cache cache (BlockAccess) to inform if the corresponding data cache
 * block needs to be written-back or not.
 *
 * @par Features
 *	* PURGE_FEATURE
 *
 * @ingroup dcache
 */
p::id<purge_t> PURGE("otawa::dcache::PURGE", INV_PURGE);


/**
 * For memory accesses not categorized as NO_PURGE, gives the a pair <M, m>
 * where M is the maximum purge time and m the minimum purge time.
 *
 * @par Features
 *	* PURGE_FEATURE
 *
 * @ingroup dcache
 */
p::id<ot::time> PURGE_TIME("otawa::dcache::PURGE_TIME", -1);

} }		// otawa::dcache
